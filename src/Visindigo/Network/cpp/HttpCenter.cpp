#include <QtCore/qdatetime.h>
#include <QtCore/qrandom.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkcookiejar.h>
#include <QtNetwork/qnetworkinformation.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtWebSockets/qwebsocket.h>
#include "General/Log.h"
#include "Network/HttpCenter.h"
#include "Network/HttpRequest.h"
#include "Network/HttpTypes.h"
#include "Network/SSERequest.h"
#include "Network/WebSocketRequest.h"
#include "Network/private/HttpCenter_p.h"
#include "Network/private/HttpReply_p.h"
#include "Network/private/WebSocketRequest_p.h"

namespace {
	bool hasHeaderCaseInsensitive(const QMap<QString, QString>& headers, const QString& key) {
		for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
			if (it.key().compare(key, Qt::CaseInsensitive) == 0) {
				return true;
			}
		}
		return false;
	}
}

namespace Visindigo::Network {
	QList<QNetworkProxy> VisindigoProxyFactory::queryProxy(const QNetworkProxyQuery& query) {
		QString key = query.peerHostName();
		if (key.isEmpty()) {
			key = query.url().host();
		}
		const auto it = Overrides.constFind(key);
		if (it != Overrides.constEnd()) {
			return { it.value() };
		}
		if (HasFallback) {
			return { Fallback };
		}
		return { QNetworkProxy(QNetworkProxy::NoProxy) };
	}

	void HttpCenterPrivate::enqueue(HttpReply* reply) {
		if (reply == nullptr) {
			return;
		}
		// 按优先级插入：数值大的排在前面，同优先级保持提交顺序，
		// 以免高优先级请求反复插队导致低优先级请求永远得不到执行。
		const qint32 priority = static_cast<qint32>(reply->d->Request.d->PriorityLevel);
		qsizetype index = 0;
		while (index < Queue.size()
			&& static_cast<qint32>(Queue.at(index)->d->Request.d->PriorityLevel) >= priority) {
			index += 1;
		}
		Queue.insert(index, reply);
	}

	void HttpCenterPrivate::release(HttpReply* reply) {
		if (reply == nullptr) {
			return;
		}
		Queue.removeAll(reply);
		Active.remove(reply->d->Id);
		pump();
	}

	/*!
		\since Visindigo 0.17.0

		把队列中的请求推入执行。之所以需要 Pumping 标志，是因为 start() 有可能
		同步失败并立即归还句柄，从而再次触发本函数；没有该标志就会递归下去。
	*/
	void HttpCenterPrivate::pump() {
		if (Pumping) {
			return;
		}
		Pumping = true;
		while (!Paused && !Queue.isEmpty() && Active.size() < MaxConcurrentRequests) {
			HttpReply* reply = Queue.takeFirst();
			Active.insert(reply->d->Id, reply);
			reply->d->start();
		}
		Pumping = false;
	}

	bool HttpCenterPrivate::isSchemeAllowed(const QUrl& url) const {
		return AllowedUrlSchemes.contains(url.scheme(), Qt::CaseInsensitive);
	}

	qint32 HttpCenterPrivate::timeoutFor(const HttpRequest& request) const {
		// 负数代表调用方明确要求关闭该项超时，与"没设过、用默认值"是两回事
		if (request.d->TimeoutMs < 0) {
			return 0;
		}
		return request.d->TimeoutMs > 0 ? request.d->TimeoutMs : DefaultTimeoutMs;
	}

	qint32 HttpCenterPrivate::idleTimeoutFor(const HttpRequest& request) const {
		if (request.d->IdleTimeoutMs < 0) {
			return 0;
		}
		return request.d->IdleTimeoutMs > 0 ? request.d->IdleTimeoutMs : DefaultIdleTimeoutMs;
	}

	RetryPolicy HttpCenterPrivate::retryFor(const HttpRequest& request) const {
		return request.d->Retry;
	}

	QString HttpCenterPrivate::userAgentFor(const HttpRequest& request) const {
		for (auto it = request.d->Headers.constBegin(); it != request.d->Headers.constEnd(); ++it) {
			if (it.key().compare(QStringLiteral("User-Agent"), Qt::CaseInsensitive) == 0) {
				return it.value();
			}
		}
		return DefaultUserAgent;
	}

	/*!
		\since Visindigo 0.17.0

		创建句柄并在此时一次性应用请求拦截器与凭据提供者。

		拦截器只在提交时执行一次，而不是每次重试都执行：重试的语义是"把同一个
		请求再发一次"，若拦截器每次都往头部追加内容，就会得到一份层层叠加的头，
		而且第二次尝试发送的内容与第一次并不相同，重试的可复现性也就无从谈起。
	*/
	HttpReply* HttpCenterPrivate::createReply(const HttpRequest& request, bool sse) {
		HttpRequest prepared = request;
		for (const auto& interceptor : RequestInterceptors) {
			interceptor(prepared);
		}
		// 请求未自行设置过策略时才套用中心默认值，否则以请求的设置为准。
		if (!prepared.d->RetryPolicySet) {
			prepared.setRetryPolicy(DefaultRetryPolicy);
		}
		if (AuthProviderHook && !hasHeaderCaseInsensitive(prepared.d->Headers,
			QStringLiteral("Authorization"))) {
			const QString credential = AuthProviderHook(prepared.d->Url);
			if (!credential.isEmpty()) {
				prepared.setHeader(QStringLiteral("Authorization"),
					QStringLiteral("Bearer ") + credential);
			}
		}
		if (LogSensitiveHeaders) {
			// 仅在显式开启时输出头部：其中可能含 Authorization 与 Cookie，
			// 默认落盘日志里不应该出现这些内容。
			QStringList headerLines;
			for (auto it = prepared.d->Headers.constBegin();
				it != prepared.d->Headers.constEnd(); ++it) {
				headerLines.append(it.key() + QStringLiteral(": ") + it.value());
			}
			vgDebug << "Visindigo::Network submit" << prepared.d->Url.toString()
				<< headerLines.join(QStringLiteral("; "));
		}
		HttpReply* reply = nullptr;
		if (sse) {
			reply = static_cast<HttpReply*>(new SSEReply());
		}
		else {
			reply = new HttpReply();
		}
		reply->d = new HttpReplyPrivate(reply, this, prepared);
		statStarted(prepared);
		return reply;
	}

	void HttpCenterPrivate::statStarted(const HttpRequest&) {
		StatTotalRequests += 1;
	}

	void HttpCenterPrivate::statFinished(const HttpResponse& response, bool failed) {
		if (failed) {
			StatTotalFailures += 1;
		}
		if (response.d->Timings.TotalMs >= 0) {
			StatTotalElapsedMs += response.d->Timings.TotalMs;
			StatSamplesWithElapsed += 1;
		}
	}

	void HttpCenterPrivate::resetStats() {
		StatTotalRequests = 0;
		StatTotalFailures = 0;
		StatTotalRetries = 0;
		StatTotalBytesReceived = 0;
		StatTotalBytesSent = 0;
		StatTotalElapsedMs = 0;
		StatSamplesWithElapsed = 0;
	}

	/*!
		\class Visindigo::Network::HttpCenter
		\inheaderfile Network/HttpCenter.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 通用 HTTP / HTTPS / WebSocket 请求中心.

		HttpCenter 是进程内单例，是本模块唯一的入口。它只补齐 Qt 没有提供的那一层，
		并不重复实现 Qt 已有的能力：代理、Cookie、TLS 配置、网络可达性判断依旧由
		Qt 的对应类承担，中心只是把它们收敛成统一的开关。

		中心真正补上的是这些：
		\list
		\li 并发上限与优先级队列，避免瞬时发起大量请求把连接池挤爆；
		\li 带退避与抖动的自动重试，并默认只对幂等方法生效；
		\li 自己实现的重定向跟随，因此可以阻止 https 降级并在跨源时剥离凭据；
		\li 流式分帧交付，格式由解码器决定，中心本身对具体协议保持中立；
		\li 全局的认证提供者与请求/响应拦截器；
		\li 累计统计，以及按标签批量取消。
		\endlist

		\warning QNetworkAccessManager 必须在其所属线程的事件循环中工作，因此本单例
		应在主线程、且 QCoreApplication 构造完成之后再创建。

		\note 给流式请求设置总超时会在时间到达后掐断长连接。长连接应当使用
		Visindigo::Network::HttpRequest::setIdleTimeoutMs() 控制空闲超时。
	*/

	/*!
		\fn void Visindigo::Network::HttpCenter::requestStarted(quint64 id, const HttpRequest& request)
		\since Visindigo 0.17.0

		请求已提交。\a request 是中心补齐解码器、凭据与超时之后的最终形态，
		而不是调用方传进来的那一份；排查问题时应当以它为准。

		\a id 请求编号。
	*/

	/*!
		\fn void Visindigo::Network::HttpCenter::requestFinished(quint64 id, const HttpResponse& response)
		\since Visindigo 0.17.0

		请求成功结束。

		\a id 请求编号。
		\a response 最终响应。
	*/

	/*!
		\fn void Visindigo::Network::HttpCenter::requestFailed(quint64 id, const HttpError& error)
		\since Visindigo 0.17.0

		请求失败。被中止的请求不计入其中，它们只走 \l{HttpReply::aborted}。

		\a id 请求编号。
		\a error 失败原因。
	*/

	/*!
		\fn void Visindigo::Network::HttpCenter::onlineStateChanged(bool online)
		\since Visindigo 0.17.0

		网络可达性发生变化。界面上的离线提示可以挂在这里，不必自己轮询。

		\a online 网络是否可达。
	*/

	/*!
		\since Visindigo 0.17.0

		取得中心实例，首次调用时创建。
	*/
	HttpCenter* HttpCenter::getInstance() {
		static HttpCenter* instance = nullptr;
		if (instance == nullptr) {
			instance = new HttpCenter();
		}
		return instance;
	}

	HttpCenter::HttpCenter() {
		d = new HttpCenterPrivate();
		d->Q = this;
		d->ProxyFactory = new VisindigoProxyFactory();
		d->NAM = new QNetworkAccessManager(this);
		d->NAM->setProxyFactory(d->ProxyFactory);
		d->DefaultUserAgent = QStringLiteral("Visindigo/%1 (Qt)")
			.arg(QStringLiteral("0.17.0"));
		// 默认不启用 Cookie 持久化：多数请求并不需要，而全局启用会让不同业务
		// 之间的登录态互相污染，是否使用应当由使用者明确决定。
		d->NAM->setCookieJar(d->CookieJarEnabled ? new QNetworkCookieJar(this) : nullptr);
		if (QNetworkInformation::loadDefaultBackend()) {
			d->NetInfo = QNetworkInformation::instance();
			if (d->NetInfo != nullptr) {
				d->Online = d->NetInfo->reachability() != QNetworkInformation::Reachability::Disconnected;
				QObject::connect(d->NetInfo, &QNetworkInformation::reachabilityChanged, this,
					[this](QNetworkInformation::Reachability reachability) {
						const bool online =
							reachability != QNetworkInformation::Reachability::Disconnected;
						if (online != d->Online) {
							d->Online = online;
							emit onlineStateChanged(online);
						}
					});
			}
		}
	}

	/*!
		\since Visindigo 0.17.0

		设置默认基础地址。使用 Visindigo::Network::HttpRequest::setPath() 指定相对
		路径的请求会基于它解析。

		\a baseUrl 默认基础地址。
	*/
	void HttpCenter::setDefaultBaseUrl(const QUrl& baseUrl) {
		d->DefaultBaseUrl = baseUrl;
	}

	/*!
		\since Visindigo 0.17.0

		返回当前设置的默认基础地址，未设置时为空。
	*/
	QUrl HttpCenter::getDefaultBaseUrl() const {
		return d->DefaultBaseUrl;
	}

	/*!
		\since Visindigo 0.17.0

		设置默认 User-Agent，请求未单独指定时使用。

		\a userAgent 默认 User-Agent 取值。
	*/
	void HttpCenter::setDefaultUserAgent(const QString& userAgent) {
		d->DefaultUserAgent = userAgent;
	}

	/*!
		\since Visindigo 0.17.0

		设置默认总超时毫秒数，默认 30000。请求可用
		Visindigo::Network::HttpRequest::setTimeoutMs() 单独覆盖。

		\note 总超时对长连接是有害的：流式响应会在时间到达后被强行掰断。
		流式请求应当改用 Visindigo::Network::HttpRequest::setIdleTimeoutMs()。

		\a ms 默认总超时毫秒数。
	*/
	void HttpCenter::setDefaultTimeoutMs(qint32 ms) {
		d->DefaultTimeoutMs = ms;
	}

	/*!
		\since Visindigo 0.17.0

		设置默认空闲超时毫秒数，即两次数据到达之间的最大间隔，默认 60000。
		请求可用 Visindigo::Network::HttpRequest::setIdleTimeoutMs() 单独覆盖。

		这是长连接唯一合适的超时形式：只要服务端持续推送数据，连接就不会被判
		超时，而长时间没有字节到达则说明链路确实出了问题。

		\a ms 默认空闲超时毫秒数。
	*/
	void HttpCenter::setDefaultIdleTimeoutMs(qint32 ms) {
		d->DefaultIdleTimeoutMs = ms;
	}

	/*!
		\since Visindigo 0.17.0

		设置默认重试策略，作为模板应用于未自行设置过策略的请求。
		请求一旦调用过 Visindigo::Network::HttpRequest::setRetryPolicy()，
		就完全以它自己的设置为准，本项不再参与。

		\a policy 默认重试策略。
	*/
	void HttpCenter::setDefaultRetryPolicy(const RetryPolicy& policy) {
		d->DefaultRetryPolicy = policy;
	}

	/*!
		\since Visindigo 0.17.0

		设置同时进行的请求数上限，默认 6。超出的请求会排队等待。

		\a count 并发请求数上限，最小为 1。
	*/
	void HttpCenter::setMaxConcurrentRequests(qint32 count) {
		d->MaxConcurrentRequests = qMax(1, count);
		d->pump();
	}

	/*!
		\since Visindigo 0.17.0

		设置等待队列长度上限，默认 128。队满时新请求会立即以
		Visindigo::Network::HttpError::ErrorType::QueueRejected 失败，
		而不是无限堆积。

		\a count 等待队列长度上限。
	*/
	void HttpCenter::setMaxQueuedRequests(qint32 count) {
		d->MaxQueuedRequests = qMax(0, count);
	}

	/*!
		\since Visindigo 0.17.0

		设置允许请求的 URL 协议白名单，默认只允许 \c https 与 \c http。

		不在白名单内的地址会在发出前就被拒绝，以
		Visindigo::Network::HttpError::ErrorType::UrlSchemeRejected 失败，而不是
		交给传输层去尝试。这可以防止调用方把用户可控的字符串直接当地址使用，
		例如 \c file: 或 \c ftp: 这类会绕过 HTTP 栈语义的协议。

		\a schemes 允许的协议白名单。
	*/
	void HttpCenter::setAllowedUrlSchemes(const QStringList& schemes) {
		d->AllowedUrlSchemes = schemes;
	}

	/*!
		\since Visindigo 0.17.0

		设置全局代理，传入空 URL 表示不使用代理。单个请求可通过
		Visindigo::Network::HttpRequest::setProxyOverride() 覆盖本设置。

		\note 代理在传输层是按主机名匹配的，因此同一个主机在同一时刻只能
		对应一个覆盖值：两个请求即使路径不同，只要是同一个主机名，就无法使用
		不同的代理。这是 Qt 的 QNetworkProxyFactory 接口本身的限制。

		\a proxy 代理地址；传空的 QUrl 表示不使用代理。
	*/
	void HttpCenter::setProxy(const QUrl& proxy) {
		d->Proxy = proxy;
		d->ProxyFactory->HasFallback = !proxy.isEmpty();
		if (!proxy.isEmpty()) {
			d->ProxyFactory->Fallback = QNetworkProxy(
				proxy.scheme() == QStringLiteral("socks5")
					? QNetworkProxy::Socks5Proxy
					: QNetworkProxy::HttpProxy,
				proxy.host(),
				static_cast<quint16>(proxy.port(0)),
				proxy.userName(),
				proxy.password());
		}
		d->NAM->setProxy(proxy.isEmpty() ? QNetworkProxy(QNetworkProxy::NoProxy)
			: d->ProxyFactory->Fallback);
	}

	/*!
		\since Visindigo 0.17.0

		设置是否启用全局 Cookie 存储，默认关闭。

		默认关闭是刻意的：Cookie 是全局共享状态，一旦自动携带，不同业务之间的
		登录态会互相污染，而且很难在事后定位“为什么这个请求带上了别人的身份”。
		需要维持会话的调用方应当显式开启，或自行管理 Cookie 头。

		\a enabled 是否启用全局 Cookie 存储。
	*/
	void HttpCenter::setCookieJarEnabled(bool enabled) {
		if (d->CookieJarEnabled == enabled) {
			return;
		}
		d->CookieJarEnabled = enabled;
		d->NAM->setCookieJar(enabled ? new QNetworkCookieJar(this) : nullptr);
	}

	/*!
		\since Visindigo 0.17.0

		设置全局是否校验证书，默认 \c true。

		全局开关与请求级 Visindigo::Network::HttpRequest::setVerifyTls() 是“与”
		关系：只有两者都为真才会校验证书。这样全局关闭可以一次性放开所有请求，
		而单个请求的关闭不会反过来影响其它请求。

		\warning 关闭它等同于放弃中间人防护，应仅在明确受控的场景下使用。

		\a enabled 全局是否校验证书。
	*/
	void HttpCenter::setCertificateVerificationEnabled(bool enabled) {
		d->CertificateVerificationEnabled = enabled;
	}

	/*!
		\since Visindigo 0.17.0

		设置请求日志是否包含敏感头部，默认关闭。

		关闭时（默认）日志只记录方法与地址；打开后会额外输出完整请求头，
		其中可能包含 Authorization 与 Cookie。因此本项只应在排查问题等临时场景下
		打开，且不应随正式构建默认启用。

		\a enabled 是否在日志中输出敏感头部。
	*/
	void HttpCenter::setLogSensitiveHeaders(bool enabled) {
		d->LogSensitiveHeaders = enabled;
	}

	/*!
		\since Visindigo 0.17.0

		设置凭据提供者。仅当请求自身没有设置 Authorization 头时，中心才调用它。

		它的价值在于把令牌刷新逻辑集中到一处：令牌快过期时只需要在一个地方续期，
		而不必在每个发起请求的地方重复实现，也不会出现“某个调用点忘了带令牌”。

		传入的地址是请求的目标地址，使提供者可以按域名返回不同服务商的凭据。

		\a provider 凭据提供者。
	*/
	void HttpCenter::setAuthProvider(AuthProvider provider) {
		d->AuthProviderHook = provider;
	}

	/*!
		\since Visindigo 0.17.0

		追加一个请求拦截器，在请求提交时被调用，可以修改请求的头、体与地址。
		多个拦截器按添加顺序执行。

		\note 拦截器只在提交时执行一次，不会在重试时重复执行。重试的语义是“把
		同一个请求再发一次”，若拦截器每次都追加内容，重试发送的内容就与首次
		不再相同，重试也就失去了可复现性。

		\a interceptor 请求拦截器。
	*/
	void HttpCenter::addRequestInterceptor(RequestInterceptor interceptor) {
		if (interceptor) {
			d->RequestInterceptors.append(interceptor);
		}
	}

	/*!
		\since Visindigo 0.17.0

		追加一个响应拦截器。它会在两个时机被调用一次：响应头到达时，以及请求
		结束时（包括失败与中止）。因此实现必须自己处理“响应尚未收齐”的情形，
		不能假设消息体已可用。

		典型用途是埋点与统一日志：把状态码、耗时、错误分类记录到一处，
		而不必在每个调用点重复写。

		\a interceptor 响应拦截器。
	*/
	void HttpCenter::addResponseInterceptor(ResponseInterceptor interceptor) {
		if (interceptor) {
			d->ResponseInterceptors.append(interceptor);
		}
	}

	/*!
		\since Visindigo 0.17.0

		清空全部请求与响应拦截器。

		主要用途是测试：在用例之间恢复干净状态，避免上一个用例注册的拦截器
		干扰下一个。
	*/
	void HttpCenter::clearInterceptors() {
		d->RequestInterceptors.clear();
		d->ResponseInterceptors.clear();
	}

	/*!
		\since Visindigo 0.17.0

		提交一个普通 HTTP 请求，立即返回句柄。请求可能仍在排队，因此不要假设
		返回时它已经发出。

		返回的句柄默认在结束后自动销毁；若希望持有它以便稍后查询结果，
		请先调用 Visindigo::Network::HttpReply::setAutoDelete()。

		\a request 要提交的请求描述。
	*/
	HttpReply* HttpCenter::request(const HttpRequest& request) {
		if (d->Queue.size() >= d->MaxQueuedRequests) {
			HttpReply* reply = d->createReply(request, false);
			HttpError error;
			error.Type = HttpError::ErrorType::QueueRejected;
			error.Message = QStringLiteral("The request queue is full");
			d->StatTotalFailures += 1;
			reply->d->failImmediately(error);
			return reply;
		}
		HttpReply* reply = d->createReply(request, false);
		d->enqueue(reply);
		emit requestStarted(reply->d->Id, request);
		d->pump();
		return reply;
	}

	/*!
		\since Visindigo 0.17.0

		以 GET 方法提交请求的便利入口。

		\a url 目标地址。
	*/
	HttpReply* HttpCenter::get(const QUrl& url) {
		HttpRequest plainRequest;
		plainRequest.setMethod(HttpRequest::Method::Get).setUrl(url);
		return this->request(plainRequest);
	}

	/*!
		\since Visindigo 0.17.0

		以 POST 方法提交 JSON 请求体的便利入口。

		\a url 目标地址。
		\a body 请求体 JSON。
	*/
	HttpReply* HttpCenter::postJson(const QUrl& url, const Visindigo::Utility::JsonConfig& body) {
		HttpRequest plainRequest;
		plainRequest.setMethod(HttpRequest::Method::Post).setUrl(url).setJsonBody(body);
		return this->request(plainRequest);
	}

	/*!
		\since Visindigo 0.17.0

		提交一个 SSE 请求。中心会自动完成三件调用方不必操心的事：挂载
		Visindigo::Network::SseDecoder、把接受类型设为 text/event-stream、
		以及在设置了续传起点时补上 Last-Event-ID 请求头。

		\note 本重载与 request(const HttpRequest&) 的区分不是可有可无的：请求入队
		必须拷贝调用方的对象，而拷贝一个 SSERequest 到 HttpRequest 必然丢失
		SSE 专属配置，所以类型必须在提交时就是明确的。

		\a request 要提交的 SSE 请求描述。
	*/
	SSEReply* HttpCenter::request(const SSERequest& request) {
		HttpRequest prepared = request.getRequest();
		prepared.setStreamDecoder(std::make_shared<SseDecoder>());
		if (!prepared.d->Headers.contains(QStringLiteral("Accept"))) {
			prepared.setAccept(QStringLiteral("text/event-stream"));
		}
		if (!prepared.d->Headers.contains(QStringLiteral("Cache-Control"))) {
			prepared.setHeader(QStringLiteral("Cache-Control"), QStringLiteral("no-cache"));
		}
		if (!request.getLastEventId().isEmpty()) {
			prepared.setHeader(QStringLiteral("Last-Event-ID"), request.getLastEventId());
		}
		if (prepared.d->IdleTimeoutMs == 0) {
			prepared.setIdleTimeoutMs(d->DefaultIdleTimeoutMs);
		}
		// SSE 断线续传复用重试机制：重试时会带上最近的事件标识重发请求。
		if (request.isAutoReconnectEnabled()) {
			RetryPolicy policy = prepared.d->Retry;
			policy.MaxAttempts = 5;
			policy.RetryOnConnectionError = true;
			policy.RetryOnTimeout = true;
			policy.RetryIdempotentOnly = false;
			policy.UseJitter = true;
			prepared.setRetryPolicy(policy);
		}
		HttpReply* reply = d->createReply(prepared, true);
		d->enqueue(reply);
		emit requestStarted(reply->d->Id, prepared);
		d->pump();
		return static_cast<SSEReply*>(reply);
	}

	/*!
		\since Visindigo 0.17.0

		建立一个 WebSocket 会话。会话与 HTTP 句柄不同，默认不会自动销毁，
		请在不再需要时明确关闭。

		\a request WebSocket 连接描述。
	*/
	WebSocketSession* HttpCenter::openWebSocket(const WebSocketRequest& request) {
		WebSocketSession* session = new WebSocketSession();
		session->d->Request = request;
		session->d->Id = d->NextId++;
		session->d->start();
		return session;
	}

	/*!
		\since Visindigo 0.17.0

		按下发时取得的编号中止请求，无论它正在排队还是已经发出。

		\a id 请求编号。
	*/
	void HttpCenter::abort(quint64 id) {
		for (HttpReply* reply : d->Queue) {
			if (reply->d->Id == id) {
				reply->abort();
				return;
			}
		}
		const auto it = d->Active.constFind(id);
		if (it != d->Active.constEnd()) {
			it.value()->abort();
		}
	}

	/*!
		\since Visindigo 0.17.0

		中止所有带指定标签的请求。典型场景是某个界面关闭时，把该界面发起的全部
		请求一并撤销，避免回调打到已经销毁的对象上。

		\a tag 请求标签。
	*/
	void HttpCenter::abortByTag(const QString& tag) {
		if (tag.isEmpty()) {
			return;
		}
		QList<HttpReply*> matches;
		for (HttpReply* reply : d->Queue) {
			if (reply->getTag() == tag) {
				matches.append(reply);
			}
		}
		for (HttpReply* reply : d->Active) {
			if (reply->getTag() == tag) {
				matches.append(reply);
			}
		}
		for (HttpReply* reply : matches) {
			reply->abort();
		}
	}

	/*!
		\since Visindigo 0.17.0

		中止全部排队中与进行中的请求。进程退出前调用它可以避免回调在对象已经
		析构之后才到达。
	*/
	void HttpCenter::abortAll() {
		const QList<HttpReply*> pending = d->Queue;
		const QList<HttpReply*> running = d->Active.values();
		for (HttpReply* reply : pending) {
			reply->abort();
		}
		for (HttpReply* reply : running) {
			reply->abort();
		}
	}

	/*!
		\since Visindigo 0.17.0

		暂停或恢复队列出队。暂停只影响尚未发出的请求，已经在传输中的请求会
		继续完成——中途掐断它们既没有必要，也会让状态更难推理。

		\a paused 是否暂停出队。
	*/
	void HttpCenter::setPaused(bool paused) {
		d->Paused = paused;
		if (!paused) {
			d->pump();
		}
	}

	/*!
		\since Visindigo 0.17.0

		返回队列是否处于暂停状态。

		return 暂停时返回 true。
	*/
	bool HttpCenter::isPaused() const {
		return d->Paused;
	}

	/*!
		\since Visindigo 0.17.0

		返回正在进行中的请求数量。
	*/
	qint32 HttpCenter::getActiveRequestCount() const {
		return static_cast<qint32>(d->Active.size());
	}

	/*!
		\since Visindigo 0.17.0

		返回等待出队的请求数量。
	*/
	qint32 HttpCenter::getQueuedRequestCount() const {
		return static_cast<qint32>(d->Queue.size());
	}

	/*!
		\since Visindigo 0.17.0

		判断当前是否具备网络可达性。该判断来自系统的网络状态通告，
		它只说明"链路可达"，不保证目标服务一定可用。

		return 系统通告网络可达时返回 true。
	*/
	bool HttpCenter::isOnline() const {
		return d->Online;
	}

	/*!
		\since Visindigo 0.17.0

		返回自启动（或上次清零）以来提交过的请求总数，包含成功与失败。

		\note 计数在请求提交时递增，因此“已提交但仍在排队”的请求也已经计入。
	*/
	quint64 HttpCenter::getTotalRequestCount() const {
		return d->StatTotalRequests;
	}

	/*!
		\since Visindigo 0.17.0

		返回累计失败的请求数。失败包含超时、连接错误、非 2xx 状态码与队列拒绝，
		但不包含调用方主动中止——主动放弃不算故障，否则统计会因正常的
		界面关闭而变得很难解读。
	*/
	quint64 HttpCenter::getTotalFailureCount() const {
		return d->StatTotalFailures;
	}

	/*!
		\since Visindigo 0.17.0

		返回累计发生的重试次数（不含首次发送）。

		它是判断服务稳定性的重要指标：失败数与请求数之比可能很低，但重试数很高，
		说明服务正在频繁抖动，只是恰好被重试策略掩盖了。
	*/
	quint64 HttpCenter::getTotalRetryCount() const {
		return d->StatTotalRetries;
	}

	/*!
		\since Visindigo 0.17.0

		返回累计接收的消息体字节数。
	*/
	quint64 HttpCenter::getTotalBytesReceived() const {
		return d->StatTotalBytesReceived;
	}

	/*!
		\since Visindigo 0.17.0

		返回累计发送的字节数。

		\note 该值由传输层上报，并非所有后端都会提供，因此在当前实现下可能恒为 0。
	*/
	quint64 HttpCenter::getTotalBytesSent() const {
		return d->StatTotalBytesSent;
	}

	/*!
		\since Visindigo 0.17.0

		返回已完成请求的平均耗时毫秒数。没有可用样本时返回 0。
	*/
	qint64 HttpCenter::getAverageElapsedMs() const {
		if (d->StatSamplesWithElapsed == 0) {
			return 0;
		}
		return d->StatTotalElapsedMs / static_cast<qint64>(d->StatSamplesWithElapsed);
	}

	/*!
		\since Visindigo 0.17.0

		清零累计统计。它不会影响进行中的请求，也不会改变实时状态计数。
	*/
	void HttpCenter::resetStatistics() {
		d->resetStats();
	}
}
