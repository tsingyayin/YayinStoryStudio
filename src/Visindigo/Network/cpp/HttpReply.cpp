#include <QtCore/qdir.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>
#include <QtCore/qsavefile.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
#include <QtCore/qurlquery.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qsslerror.h>
#include <QtNetwork/qsslconfiguration.h>
#include "General/Log.h"
#include "Network/HttpRequest.h"
#include "Network/HttpTypes.h"
#include "Network/private/HttpCenter_p.h"
#include "Network/private/HttpReply_p.h"

namespace {
	// 回调糖的 context 允许为空，此时退化为"跟随句柄本身"的生命周期，
	// 既不会造成悬垂指针，也不会让连接永远活着。
	QObject* effectiveContext(QObject* context, QObject* fallback) {
		return context != nullptr ? context : fallback;
	}

	// 重定向时是否需要把请求体与方法一起改写。
	// 303 一律转 GET；301/302 在历史上被广泛实现为把 POST 转 GET；307/308 必须原样保留。
	bool redirectNeedsMethodChange(qint32 status, Visindigo::Network::HttpRequest::Method method) {
		if (status == 303) {
			return method != Visindigo::Network::HttpRequest::Method::Head;
		}
		if (status == 301 || status == 302) {
			return method == Visindigo::Network::HttpRequest::Method::Post;
		}
		return false;
	}

	QByteArray methodName(Visindigo::Network::HttpRequest::Method method) {
		using M = Visindigo::Network::HttpRequest::Method;
		switch (method) {
		case M::Get: return QByteArrayLiteral("GET");
		case M::Head: return QByteArrayLiteral("HEAD");
		case M::Post: return QByteArrayLiteral("POST");
		case M::Put: return QByteArrayLiteral("PUT");
		case M::Patch: return QByteArrayLiteral("PATCH");
		case M::Delete: return QByteArrayLiteral("DELETE");
		case M::Options: return QByteArrayLiteral("OPTIONS");
		default: return QByteArrayLiteral("GET");
		}
	}

	bool isIdempotent(Visindigo::Network::HttpRequest::Method method) {
		using M = Visindigo::Network::HttpRequest::Method;
		switch (method) {
		case M::Get:
		case M::Head:
		case M::Put:
		case M::Delete:
		case M::Options:
			return true;
		default:
			return false;
		}
	}

	// 方法名会被原样拼进请求行，因此必须限定为 RFC 7230 定义的 token，
	// 否则一个含空格或换行的字符串就能伪造出额外的请求头。
	bool isValidHttpToken(const QByteArray& verb) {
		if (verb.isEmpty()) {
			return false;
		}
		for (const char raw : verb) {
			const bool allowed = (raw >= 'A' && raw <= 'Z') || (raw >= 'a' && raw <= 'z')
				|| (raw >= '0' && raw <= '9')
				|| raw == '!' || raw == '#' || raw == '$' || raw == '%' || raw == '&'
				|| raw == '\'' || raw == '*' || raw == '+' || raw == '-' || raw == '.'
				|| raw == '^' || raw == '_' || raw == '`' || raw == '|' || raw == '~';
			if (!allowed) {
				return false;
			}
		}
		return true;
	}
}

namespace Visindigo::Network {
	/*!
		\class Visindigo::Network::HttpReply
		\inheaderfile Network/HttpRequest.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 普通 HTTP 请求的句柄.

		HttpReply 是调用方与一次请求交互的窗口。它由
		Visindigo::Network::HttpCenter 在提交请求时创建并返回，调用方只持有指针，
		无法自行构造。

		底层传输天然是字节流，"一次性请求""文件下载""文本流"在本模块里都是同一个
		类，差别只在请求上设置的 \l HttpRequest::BodySink 与
		Visindigo::Network::HttpRequest::setStreamDecoder()：

		\list
		\li \l{HttpRequest::BodySink::Auto}、\l{HttpRequest::BodySink::Buffer}：
			请求结束后从 \l getResponse() 的 \l HttpResponse::getBody() 取走内容。
		\li \l{HttpRequest::BodySink::ToFile}：内容增量写入磁盘，
			\l downloadProgress 可作为进度显示，文件路径见
			Visindigo::Network::HttpResponse::getDownloadFilePath()。
		\li 挂了解码器时：字节按帧切分，经 \l frameReceived 交付。
		\endlist

		除信号之外还提供一组回调糖（\l{then}、\l{onError}、\l{onFrame} 等），
		它们等价于连接对应信号，只是为了链式书写与集中阅读：
		\code
			center->request(request)
			      ->then(this, [](const HttpResponse& response) { ... })
			      ->onError(this, [](const HttpError& error) { ... });
		\endcode
		带 context 的重载遵循 Qt 惯例，context 销毁后连接自动断开，因此不会在对象
		已销毁之后回调。但要注意"不再回调"并不等于"连接已关闭"，长连接场景应当
		使用 \l bindLifecycleTo() 才能真正把请求停掉。

		\note 请求结束时若 \l isAutoDelete() 为 \c true （默认值），句柄会自动
		deleteLater，因此不要在提交后长期保存这个指针；需要事后查询结果时，
		应先调用 \c{setAutoDelete(false)} 并自行负责释放。

		\note 对设置了流式解码器的请求，\l then 的语义是"流正常结束"，而不是
		"已经拿到了全部内容"——内容是在 \l frameReceived 中陆续交付的。

		\sa Visindigo::Network::HttpRequest
		\sa Visindigo::Network::HttpResponse
		\sa Visindigo::Network::SSEReply
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::queued(quint64 id)
		\since Visindigo 0.17.0

		请求已进入中心队列但尚未发出。\a id 是句柄编号，与 \l getId() 一致。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::started(quint64 id)
		\since Visindigo 0.17.0

		请求真正被送出。排队中的请求不会发这个信号，因此"已排队"与"已发出"
		是两个可以区分的状态。

		\a id 句柄编号。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::headersReceived(const HttpResponse& response)
		\since Visindigo 0.17.0

		响应头到达。此时 \a response 的 \l HttpResponse::isValid() 为 \c true，
		但消息体尚未入场，\l HttpResponse::getBody() 还是空的。

		这是下载开始之前校验状态码、并据此决定要不要 \l abort() 的唯一时机。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::uploadProgress(qint64 sent, qint64 total)
		\since Visindigo 0.17.0

		上传进度。\a total 为 \c -1 表示长度未知。

		\a sent 已发送字节数。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::downloadProgress(qint64 received, qint64 total)
		\since Visindigo 0.17.0

		下载进度。\a total 为 \c -1 表示长度未知，分块传输与流式响应都属此列。

		\a received 已接收字节数。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::retrying(qint32 attempt, qint32 delayMs, const HttpError& lastError)
		\since Visindigo 0.17.0

		即将重试。\a attempt 是第几次重试（从 1 开始），\a delayMs 是即将等待的
		毫秒数，\a lastError 是触发重试的那次失败。

		\note 重试的是整个请求，请求体会被重新发送，这是默认只对幂等方法重试的原因。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::bodyChunkReceived(const QByteArray& chunk)
		\since Visindigo 0.17.0

		未被缓存到内存的原始增量字节。

		\note \a chunk 的切分位置没有任何语义，不能假定它与协议边界对齐；
		需要按帧处理时请使用 \l{frameReceived}。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::frameReceived(const StreamFrame& frame)
		\since Visindigo 0.17.0

		解码器切出的完整帧。仅在请求挂了 \l Visindigo::Network::IStreamDecoder 时发出。

		\a frame 已切出的完整帧。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::succeeded(const HttpResponse& response)
		\since Visindigo 0.17.0

		请求成功结束。\a response 是最终结果。

		\note 它只说明这次 HTTP 交互成功（收到了 2xx 且未被中止），不代表业务成功。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::failed(const HttpError& error)
		\since Visindigo 0.17.0

		请求失败。\a error 给出分类与原因，是否值得重试见其 \c Retryable 字段。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::finished(const HttpResponse& response)
		\since Visindigo 0.17.0

		请求结束，成功与失败都会发出。"无论如何都要收尾"的逻辑（例如解除加载状态）
		挂在这里，比同时连接 \l succeeded 与 \l failed 更不容易漏掉分支。

		\a response 最终响应。
	*/

	/*!
		\fn void Visindigo::Network::HttpReply::aborted()
		\since Visindigo 0.17.0

		请求被中止。主动 \l abort() 与上下文销毁导致的自动中止都会发出它。

		\note 中止之后不会再发出 \l{succeeded} 或 \l{failed}。
	*/

	HttpReply::HttpReply() : d(nullptr) {
	}

	HttpReply::~HttpReply() {
		delete d;
	}

	HttpReplyPrivate::HttpReplyPrivate(HttpReply* owner, HttpCenterPrivate* center, const HttpRequest& request) {
		Q = owner;
		Center = center;
		Request = request;
		Id = center != nullptr ? center->NextId++ : 0;
		TraceId = QStringLiteral("vi-%1").arg(Id, 8, 16, QLatin1Char('0'));
		Response.d->TraceId = TraceId;
		// 不在此处预填落盘路径：只有真的写入成功后才由 finalizeSuccess() 填写，
		// 否则调用方会在请求失败时看到一个非空路径，误以为文件已存好。
	}

	HttpReplyPrivate::~HttpReplyPrivate() {
		cleanupTransport();
		delete RetryTimer;
		delete TotalTimer;
		delete IdleTimer;
		delete Sink;
	}

	/*!
		\since Visindigo 0.17.0

		断开并销毁当前的传输对象。重试与重定向都需要先把上一跳彻底拆干净，
		否则残留的 QNetworkReply 会继续把数据推给已经决定放弃的本次尝试。
	*/
	void HttpReplyPrivate::cleanupTransport() {
		if (TotalTimer != nullptr) {
			TotalTimer->stop();
		}
		if (IdleTimer != nullptr) {
			IdleTimer->stop();
		}
		if (!Transport.isNull()) {
			QNetworkReply* transport = Transport.data();
			Transport.clear();
			transport->disconnect(Q);
			transport->abort();
			transport->deleteLater();
		}
		if (Sink != nullptr) {
			Sink->cancelWriting();
			delete Sink;
			Sink = nullptr;
		}
		if (Center != nullptr && !Request.d->ProxyOverride.isEmpty()) {
			Center->ProxyFactory->Overrides.remove(CurrentUrl.host());
		}
	}

	/*!
		\since Visindigo 0.17.0

		开始一次尝试。它同时服务于首次发送、重试与重定向跳转：三者对传输层而言
		都是"重新发一次请求"，差别只在方法与地址是否已被改写。
	*/
	void HttpReplyPrivate::start() {
		Attempt += 1;
		AbortRequested = false;
		Completed = false;
		State = HttpReply::State::Running;
		Response.d->Error = HttpError();
		if (!Elapsed.isValid()) {
			Elapsed.start();
		}
		emit Q->started(Id);

		// 目标地址：首次尝试时结合中心的基础地址解析，之后跟随重定向改写。
		if (Attempt == 1 && RedirectCount == 0) {
			CurrentUrl = Request.d->buildFinalUrl(Center->DefaultBaseUrl);
			CurrentMethod = Request.d->MethodName;
			CurrentBody = Request.d->buildFinalBody(&CurrentContentType);
			Response.d->FinalUrl = CurrentUrl;
		}

		if (!Center->isSchemeAllowed(CurrentUrl)) {
			HttpError error;
			error.Type = HttpError::ErrorType::UrlSchemeRejected;
			error.Message = QStringLiteral("URL scheme is not allowed: %1").arg(CurrentUrl.scheme());
			finalizeFailure(error, false);
			return;
		}

		QNetworkRequest netRequest(CurrentUrl);
		// 自己处理重定向：只有这样才拿得到完整跳转链，并对每一跳做降级与凭据检查。
		netRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
			QNetworkRequest::ManualRedirectPolicy);
		netRequest.setTransferTimeout(0);

		for (auto it = Request.d->Headers.constBegin(); it != Request.d->Headers.constEnd(); ++it) {
			netRequest.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
		}
		if (!CurrentContentType.isEmpty() && !netRequest.hasRawHeader("Content-Type")) {
			netRequest.setRawHeader(QByteArrayLiteral("Content-Type"), CurrentContentType);
		}
		if (!netRequest.hasRawHeader("User-Agent")) {
			netRequest.setRawHeader(QByteArrayLiteral("User-Agent"),
				Center->userAgentFor(Request).toUtf8());
		}
		// 全局开关与请求级开关是与关系：全局关闭可一次性放开所有请求，
		// 而单个请求的关闭不会反过来影响其它请求。
		VerifyTlsEffective = Request.d->VerifyTls
			&& (Center == nullptr || Center->CertificateVerificationEnabled);
		if (!VerifyTlsEffective) {
			QSslConfiguration configuration = netRequest.sslConfiguration();
			configuration.setPeerVerifyMode(QSslSocket::VerifyNone);
			netRequest.setSslConfiguration(configuration);
		}
		if (!Request.d->ProxyOverride.isEmpty()) {
			Center->ProxyFactory->Overrides.insert(CurrentUrl.host(),
				QNetworkProxy(Request.d->ProxyOverride.scheme() == QStringLiteral("socks5")
					? QNetworkProxy::Socks5Proxy
					: QNetworkProxy::HttpProxy,
					Request.d->ProxyOverride.host(),
					static_cast<quint16>(Request.d->ProxyOverride.port(0))));
		}

		const QByteArray verb = CurrentMethod == HttpRequest::Method::Custom
			&& isValidHttpToken(Request.d->CustomMethod)
			? Request.d->CustomMethod
			: methodName(CurrentMethod);
		QNetworkReply* transport = CurrentBody.isEmpty()
			? Center->NAM->sendCustomRequest(netRequest, verb)
			: Center->NAM->sendCustomRequest(netRequest, verb, CurrentBody);
		Transport = transport;

		QObject::connect(transport, &QNetworkReply::metaDataChanged, Q, [this]() {
			onMetaDataChanged();
		});
		QObject::connect(transport, &QNetworkReply::readyRead, Q, [this]() {
			onReadyRead();
		});
		QObject::connect(transport, &QNetworkReply::uploadProgress, Q,
			[this](qint64 sent, qint64 total) {
				if (!AbortRequested) {
					Q->uploadProgress(sent, total);
				}
			});
		QObject::connect(transport, &QNetworkReply::sslErrors, Q,
			[this](const QList<QSslError>& errors) {
				if (!VerifyTlsEffective && !Transport.isNull()) {
					// 仅当全局与请求级都允许时才忽略；否则交给传输层失败，
					// 使证书问题成为一个可见的错误而不是静默降级的安全漏洞。
					Transport->ignoreSslErrors(errors);
				}
			});
		QObject::connect(transport, &QNetworkReply::finished, Q, [this]() {
			onTransportFinished();
		});

		const qint32 totalMs = Center->timeoutFor(Request);
		if (totalMs > 0) {
			if (TotalTimer == nullptr) {
				TotalTimer = new QTimer(Q);
				TotalTimer->setSingleShot(true);
				QObject::connect(TotalTimer, &QTimer::timeout, Q, [this]() {
					HttpError error;
					error.Type = HttpError::ErrorType::Timeout;
					error.Message = QStringLiteral("Request exceeded the total timeout");
					finalizeFailure(error, true);
				});
			}
			TotalTimer->start(totalMs);
		}
		const qint32 idleMs = Center->idleTimeoutFor(Request);
		if (idleMs > 0) {
			if (IdleTimer == nullptr) {
				IdleTimer = new QTimer(Q);
				IdleTimer->setSingleShot(true);
				QObject::connect(IdleTimer, &QTimer::timeout, Q, [this]() {
					HttpError error;
					error.Type = HttpError::ErrorType::IdleTimeout;
					error.Message = QStringLiteral("No data received within the idle timeout");
					finalizeFailure(error, true);
				});
			}
			IdleTimer->start(idleMs);
		}
	}

	/*!
		\since Visindigo 0.17.0

		响应头到达。此时状态码已经可以读到，因此要在这里决定消息体的去向：
		3xx 的跳转提示页与 4xx/5xx 的错误页都不允许写入下载目标，避免调用方
		拿到一个"下载成功"的错误页文件。
	*/
	void HttpReplyPrivate::onMetaDataChanged() {
		if (AbortRequested || Transport.isNull()) {
			return;
		}
		const qint32 status = Transport->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (status == 0 || State != HttpReply::State::Running) {
			return;
		}
		State = HttpReply::State::ReceivingBody;

		Response.d->Valid = true;
		Response.d->StatusCode = status;
		Response.d->ReasonPhrase = Transport->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
		Response.d->Headers.clear();
		for (const auto& pair : Transport->rawHeaderPairs()) {
			Response.d->Headers.insert(QString::fromLatin1(pair.first).toLower(),
				QString::fromUtf8(pair.second));
		}
		Response.d->FinalUrl = Transport->url();
		const bool hasLength = Response.d->Headers.contains(QStringLiteral("content-length"));
		Response.d->TotalBytes = hasLength
			? Response.d->Headers.value(QStringLiteral("content-length")).toLongLong()
			: -1;

		const bool redirecting = status >= 300 && status < 400 && status != 304;
		const bool willFollow = redirecting
			&& Request.d->RedirectMode != HttpRequest::RedirectPolicy::Never;
		const bool noBodyExpected = status == 204 || status == 304
			|| Request.d->MethodName == HttpRequest::Method::Head;
		if (willFollow) {
			// 中间跳转的响应体对调用方没有意义，直接丢弃，既不占内存也不落盘。
			Mode = BodyMode::Discard;
		}
		else if (redirecting || status >= 400 || noBodyExpected) {
			// 交给调用方的 3xx、错误体，以及按规范保证无消息体的状态（204/304/HEAD）：
			// 一律只进内存。否则会把跳转提示页或 0 字节文件当成下载结果交出去。
			Mode = BodyMode::Buffer;
		}
		else if (Request.d->SinkKind == HttpRequest::BodySink::ToFile) {
			Mode = BodyMode::ToFile;
			const QString path = Request.d->DownloadFilePath;
			HttpError error;
			error.Type = HttpError::ErrorType::LocalIoError;
			if (path.isEmpty()) {
				// 不能静默跳过：那样字节会被丢弃而请求还报告成功，
				// 调用方会以为文件已经存好，这是最坏的一种失败方式。
				error.Message = QStringLiteral("ToFile sink requires a download file path");
				finalizeFailure(error, false);
				return;
			}
			const QFileInfo info(path);
			if (!info.absoluteDir().exists() && !QDir().mkpath(info.absolutePath())) {
				error.Message = QStringLiteral("Cannot create directory for the download target");
				error.Detail = info.absolutePath();
				finalizeFailure(error, false);
				return;
			}
			Sink = new QSaveFile(path);
			if (!Sink->open(QIODevice::WriteOnly)) {
				delete Sink;
				Sink = nullptr;
				error.Message = QStringLiteral("Cannot open download target for writing");
				error.Detail = path;
				finalizeFailure(error, false);
				return;
			}
		}
		else {
			Mode = BodyMode::Buffer;
		}

		// 请求级拦截器在这里作用于已到达的响应头，调用方可以据此改写状态判断。
		if (Center != nullptr) {
			for (const auto& interceptor : Center->ResponseInterceptors) {
				interceptor(Response);
			}
		}
		emit Q->headersReceived(Response);
	}

	/*!
		\since Visindigo 0.17.0

		读取本次到达的字节。这是唯一会把数据交付出去的地方：内存缓存、落盘、
		流式分帧三条路径都由此分派。
	*/
	void HttpReplyPrivate::onReadyRead() {
		if (AbortRequested || Transport.isNull()) {
			return;
		}
		if (!FirstByteSeen) {
			FirstByteSeen = true;
			Response.d->Timings.TtfbMs = Elapsed.elapsed();
		}
		if (IdleTimer != nullptr) {
			IdleTimer->start(Center->idleTimeoutFor(Request));
		}
		consumeChunk(Transport->readAll());
	}

	void HttpReplyPrivate::consumeChunk(const QByteArray& chunk) {
		if (chunk.isEmpty()) {
			return;
		}
		Response.d->ReceivedBytes += chunk.size();
		emit Q->downloadProgress(Response.d->ReceivedBytes, Response.d->TotalBytes);

		if (Mode != BodyMode::Discard) {
			emit Q->bodyChunkReceived(chunk);
		}
		// 挂解码器与字节去向无关：落盘的同时也可能需要按帧消费（例如下载大体积
		// NDJSON），所以这里不能限定 Buffer 模式，否则 ToFile 下它永远不会生效。
		if (!Decoder && Request.d->Decoder && Mode != BodyMode::Discard) {
			Decoder = Request.d->Decoder;
			Decoder->setMaxFrameBytes(Request.d->MaxFrameBytes);
		}

		switch (Mode) {
		case BodyMode::Discard:
			break;
		case BodyMode::Buffer: {
			const qint64 limit = qMax<qint64>(Request.d->MaxBufferBytes, 1);
			if (Response.d->Body.size() + chunk.size() > limit) {
				HttpError error;
				error.Type = HttpError::ErrorType::ResponseTooLarge;
				error.Message = QStringLiteral("Response body exceeded the in-memory limit");
				error.Retryable = false;
				finalizeFailure(error, false);
				return;
			}
			Response.d->Body.append(chunk);
			break;
		}
		case BodyMode::ToFile: {
			if (Request.d->MaxResponseBytes > 0
				&& SinkBytes + chunk.size() > Request.d->MaxResponseBytes) {
				HttpError error;
				error.Type = HttpError::ErrorType::ResponseTooLarge;
				error.Message = QStringLiteral("Download exceeded the configured size limit");
				finalizeFailure(error, false);
				return;
			}
			if (Sink != nullptr && Sink->write(chunk) != chunk.size()) {
				HttpError error;
				error.Type = HttpError::ErrorType::Unknown;
				error.Message = QStringLiteral("Failed to write downloaded data");
				finalizeFailure(error, false);
				return;
			}
			SinkBytes += chunk.size();
			break;
		}
		default:
			break;
		}

		if (Decoder) {
			feedDecoder(chunk, false);
		}
	}

	/*!
		\since Visindigo 0.17.0

		把字节交给解码器并按帧发出。解码器是唯一处理分帧的地方，因此这里对具体
		格式（SSE、NDJSON 或调用方自定义）完全中立。
	*/
	void HttpReplyPrivate::feedDecoder(const QByteArray& chunk, bool finishing) {
		if (!Decoder || DecoderFinished) {
			return;
		}
		QList<StreamFrame> frames = finishing ? Decoder->finish() : Decoder->feed(chunk);
		for (const StreamFrame& frame : frames) {
			if (!frame.isValid()) {
				continue;
			}
			emit Q->frameReceived(frame);
			if (AbortRequested) {
				return;
			}
		}
		if (finishing) {
			DecoderFinished = true;
		}
	}

	/*!
		\since Visindigo 0.17.0

		一次尝试结束。三种去向：跟随重定向、因错误重试、或者收敛为最终结果。
	*/
	void HttpReplyPrivate::onTransportFinished() {
		if (AbortRequested || Completed) {
			return;
		}
		QNetworkReply* transport = Transport.data();
		if (transport == nullptr) {
			return;
		}
		consumeChunk(transport->readAll());
		if (Completed) {
			return;
		}
		if (!Response.d->Valid) {
			// 响应头始终未到达，补一次元数据解析（部分错误路径不会触发 metaDataChanged）
			onMetaDataChanged();
			if (Completed) {
				return;
			}
		}

		const qint32 status = Response.d->StatusCode;
		const QNetworkReply::NetworkError transportError = transport->error();

		// 重定向
		const bool redirecting = status >= 300 && status < 400 && status != 304;
		if (redirecting && Request.d->RedirectMode != HttpRequest::RedirectPolicy::Never) {
			const QString location = Response.d->Headers.value(QStringLiteral("location"));
			if (!location.isEmpty()) {
				if (RedirectCount >= Request.d->FollowRedirectsMax) {
					HttpError error;
					error.Type = HttpError::ErrorType::TooManyRedirects;
					error.Message = QStringLiteral("Redirect limit exceeded");
					error.HttpStatus = status;
					finalizeFailure(error, false);
					return;
				}
				QUrl next = CurrentUrl.resolved(QUrl(location));
				if (Request.d->RedirectMode == HttpRequest::RedirectPolicy::Safe) {
					if (CurrentUrl.scheme() == QStringLiteral("https")
						&& next.scheme() == QStringLiteral("http")) {
						HttpError error;
						error.Type = HttpError::ErrorType::ProtocolError;
						error.Message = QStringLiteral("Redirect attempted to downgrade https to http");
						error.HttpStatus = status;
						finalizeFailure(error, false);
						return;
					}
					if (next.host().compare(CurrentUrl.host(), Qt::CaseInsensitive) != 0
						|| next.port(0) != CurrentUrl.port(0)) {
						// 跨源跳转必须剥离凭据，否则令牌会被转发给第三方主机。
						for (const QString& key : { QStringLiteral("Authorization"),
												   QStringLiteral("Cookie") }) {
							Request.d->Headers.remove(key);
						}
					}
				}
				Response.d->RedirectChain.append(CurrentUrl);
				RedirectCount += 1;
				CurrentUrl = next;
				if (redirectNeedsMethodChange(status, CurrentMethod)) {
					CurrentMethod = HttpRequest::Method::Get;
					CurrentBody.clear();
					CurrentContentType.clear();
				}
				cleanupTransport();
				State = HttpReply::State::Running;
				Response.d->Valid = false;
				Response.d->Headers.clear();
				Response.d->StatusCode = 0;
				Response.d->Body.clear();
				Response.d->ReceivedBytes = 0;
				SinkBytes = 0;
				DecoderFinished = false;
				if (Decoder) {
					Decoder->reset();
				}
				start();
				return;
			}
			// 能走到这里说明调用方要求跟随重定向，服务端却没给出下一跳地址，
			// 属于协议层面的异常。不能当成功处理。
			HttpError error;
			error.Type = HttpError::ErrorType::ProtocolError;
			error.HttpStatus = status;
			error.Message = QStringLiteral("Redirect response carries no Location header");
			finalizeFailure(error, false);
			return;
		}

		// 传输层错误（不含"HTTP 状态码为错误码"这一情形）
		if (transportError != QNetworkReply::NoError && status == 0) {
			const HttpError error = mapTransportError();
			if (error.Type == HttpError::ErrorType::Aborted) {
				AbortRequested = true;
				finalizeFailure(error, false);
				return;
			}
			finalizeFailure(error, true);
			return;
		}

		// HTTP 状态码层面的失败
		if (status >= 400) {
			HttpError error;
			error.Type = HttpError::ErrorType::HttpStatusError;
			error.HttpStatus = status;
			error.Message = QStringLiteral("Server returned HTTP %1 %2")
				.arg(status).arg(Response.d->ReasonPhrase);
			error.Detail = QString::fromUtf8(Response.d->Body);
			if (error.Detail.size() > 4096) {
				error.Detail.truncate(4096);
			}
			error.Retryable = Request.d->Retry.RetryOnStatusError
				&& Request.d->Retry.RetryableStatusCodes.contains(status);
			error.SuggestedRetryDelayMs = Response.d->Headers
				.value(QStringLiteral("retry-after")).toInt();
			finalizeFailure(error, true);
			return;
		}

		finalizeSuccess();
	}

	/*!
		\since Visindigo 0.17.0

		把 Qt 的传输错误映射为本模块的错误分类。分类的意义在于让重试策略能够
		按"值得重试"与"不该重试"区分，例如证书错误重试多少次都不会变好。
	*/
	HttpError HttpReplyPrivate::mapTransportError() const {
		HttpError error;
		if (Transport.isNull()) {
			error.Type = HttpError::ErrorType::Unknown;
			return error;
		}
		const auto code = Transport->error();
		switch (code) {
		case QNetworkReply::OperationCanceledError:
			error.Type = HttpError::ErrorType::Aborted;
			break;
		case QNetworkReply::TimeoutError:
			error.Type = HttpError::ErrorType::Timeout;
			break;
		case QNetworkReply::ConnectionRefusedError:
			error.Type = HttpError::ErrorType::ConnectionRefused;
			break;
		case QNetworkReply::HostNotFoundError:
			error.Type = HttpError::ErrorType::HostNotFound;
			break;
		case QNetworkReply::NetworkSessionFailedError:
		case QNetworkReply::UnknownNetworkError:
			error.Type = HttpError::ErrorType::NetworkUnreachable;
			break;
		case QNetworkReply::ProxyConnectionRefusedError:
		case QNetworkReply::ProxyConnectionClosedError:
		case QNetworkReply::ProxyNotFoundError:
		case QNetworkReply::ProxyTimeoutError:
		case QNetworkReply::ProxyAuthenticationRequiredError:
			error.Type = HttpError::ErrorType::ProxyError;
			break;
		case QNetworkReply::SslHandshakeFailedError:
			error.Type = HttpError::ErrorType::TlsError;
			break;
		case QNetworkReply::ProtocolUnknownError:
		case QNetworkReply::ProtocolInvalidOperationError:
		case QNetworkReply::ProtocolFailure:
			error.Type = HttpError::ErrorType::ProtocolError;
			break;
		default:
			error.Type = HttpError::ErrorType::Unknown;
			break;
		}
		error.Message = Transport->errorString();
		if (Response.d->StatusCode != 0) {
			error.HttpStatus = Response.d->StatusCode;
		}
		switch (error.Type) {
		case HttpError::ErrorType::Timeout:
			error.Retryable = Request.d->Retry.RetryOnTimeout;
			break;
		case HttpError::ErrorType::TlsError:
		case HttpError::ErrorType::CertificateError:
		case HttpError::ErrorType::Aborted:
		case HttpError::ErrorType::ProtocolError:
		case HttpError::ErrorType::HostNotFound:
		case HttpError::ErrorType::UrlSchemeRejected:
			error.Retryable = false;
			break;
		default:
			error.Retryable = Request.d->Retry.RetryOnConnectionError;
			break;
		}
		return error;
	}

	/*!
		\since Visindigo 0.17.0

		判断当前失败是否还有重试的机会。除了策略本身的开关，还必须检查是否已经
		用尽尝试次数，以及方法是否幂等——对非幂等方法重试可能让服务端重复处理。
	*/
	bool HttpReplyPrivate::shouldRetry(const HttpError& error) const {
		const RetryPolicy& policy = Request.d->Retry;
		if (policy.MaxAttempts <= 1) {
			return false;
		}
		if (Attempt >= policy.MaxAttempts) {
			return false;
		}
		if (policy.RetryIdempotentOnly && !isIdempotent(Request.d->MethodName)) {
			return false;
		}
		if (AbortRequested) {
			return false;
		}
		return error.Retryable;
	}

	/*!
		\since Visindigo 0.17.0

		计算下一次重试前应等待的毫秒数。始终以服务端给出的 Retry-After 为最高优先，
		因为服务端清楚自己何时能够恢复；否则按指数退避，并可叠加抖动以避免
		大量客户端在同一时刻同时重连。
	*/
	qint32 HttpReplyPrivate::retryDelayFor(qint32 attempt, const RetryPolicy& policy, qint32 suggestedMs) {
		if (policy.RespectRetryAfter && suggestedMs > 0) {
			return suggestedMs * 1000;
		}
		qreal delay = policy.InitialDelayMs * qPow(policy.BackoffMultiplier, qMax(0, attempt - 1));
		if (policy.UseJitter && delay > 0) {
			const qreal factor = 0.5 + (QRandomGenerator::global()->generateDouble() * 0.5);
			delay *= factor;
		}
		const qint32 bounded = static_cast<qint32>(qBound<qreal>(0, delay, policy.MaxDelayMs));
		return qMax(bounded, 0);
	}

	/*!
		\since Visindigo 0.17.0

		安排一次重试。退避等待期间句柄仍处于占用状态，因此它不会因为等待而
		被其它请求挤掉优先级。
	*/
	void HttpReplyPrivate::scheduleRetry(const HttpError& error) {
		const qint32 delay = retryDelayFor(Attempt, Request.d->Retry, error.SuggestedRetryDelayMs);
		cleanupTransport();
		RetryCount += 1;
		if (Center != nullptr) {
			Center->StatTotalRetries += 1;
		}
		if (RetryTimer == nullptr) {
			RetryTimer = new QTimer(Q);
			RetryTimer->setSingleShot(true);
			QObject::connect(RetryTimer, &QTimer::timeout, Q, [this]() {
				if (!AbortRequested) {
					start();
				}
			});
		}
		Response.d->Error = error;
		emit Q->retrying(Attempt, delay, error);
		RetryTimer->start(delay);
	}

	/*!
		\since Visindigo 0.17.0

		用于请求连入队都未能完成的情形（典型情况是等待队列已满）。
		因为信号由句柄自己发出，所以不能由中心代劳——Qt 的信号是受保护成员，
		从外部发射另一个对象的信号不是合法写法。
	*/
	void HttpReplyPrivate::failImmediately(const HttpError& error) {
		Response.d->Error = error;
		Completed = true;
		State = HttpReply::State::Finished;
		Center = nullptr;
		emit Q->failed(error);
		emit Q->finished(Response);
		if (AutoDelete) {
			Q->deleteLater();
		}
	}

	void HttpReplyPrivate::finalizeSuccess() {		if (Sink != nullptr) {
			if (!Sink->commit()) {
				HttpError error;
				error.Type = HttpError::ErrorType::Unknown;
				error.Message = QStringLiteral("Failed to finalize the downloaded file");
				finalizeFailure(error, false);
				return;
			}
			delete Sink;
			Sink = nullptr;
			Response.d->DownloadFilePath = Request.d->DownloadFilePath;
		}
		if (Decoder && !DecoderFinished) {
			feedDecoder(QByteArray(), true);
		}
		Response.d->Timings.TotalMs = Elapsed.elapsed();
		Response.d->Error = HttpError();
		Completed = true;
		State = HttpReply::State::Finished;
		cleanupTransport();

		if (Center != nullptr) {
			Center->statFinished(Response, false);
			Center->StatTotalBytesReceived += static_cast<quint64>(Response.d->ReceivedBytes);
		}
		emitFinishedSignals();
		emit Q->succeeded(Response);
		emit Q->finished(Response);

		if (Center != nullptr) {
			HttpCenterPrivate* center = Center;
			Center = nullptr;
			center->release(Q);
		}
		if (AutoDelete) {
			Q->deleteLater();
		}
	}

	void HttpReplyPrivate::finalizeFailure(const HttpError& error, bool allowRetry) {
		Response.d->Error = error;
		if (allowRetry && shouldRetry(error)) {
			scheduleRetry(error);
			return;
		}
		if (Sink != nullptr) {
			// 未 commit 的 QSaveFile 在销毁时会自动丢弃临时文件，
			// 因此失败不会在目标路径留下半个文件。
			delete Sink;
			Sink = nullptr;
		}
		Response.d->Timings.TotalMs = Elapsed.elapsed();
		Completed = true;
		State = AbortRequested ? HttpReply::State::Aborted : HttpReply::State::Finished;
		cleanupTransport();

		if (Center != nullptr) {
			Center->statFinished(Response, true);
		}
		emitFinishedSignals();
		if (AbortRequested) {
			emit Q->aborted();
		}
		else {
			emit Q->failed(error);
		}
		emit Q->finished(Response);

		if (Center != nullptr) {
			HttpCenterPrivate* center = Center;
			Center = nullptr;
			center->release(Q);
		}
		if (AutoDelete) {
			Q->deleteLater();
		}
	}

	/*!
		\since Visindigo 0.17.0

		在结束前把请求级响应拦截器跑一遍，让埋点与日志能观察到最终结果，
		包括失败的结果。
	*/
	void HttpReplyPrivate::emitFinishedSignals() {
		if (Center == nullptr) {
			return;
		}
		for (const auto& interceptor : Center->ResponseInterceptors) {
			interceptor(Response);
		}
	}

	/*!
		\since Visindigo 0.17.0

		返回句柄的编号。它在进程内唯一，可用于日志关联，也是
		Visindigo::Network::HttpCenter::abort() 的入参。
	*/
	quint64 HttpReply::getId() const {
		return d->Id;
	}

	/*!
		\since Visindigo 0.17.0

		返回请求携带的标签，未设置时为空字符串。
	*/
	QString HttpReply::getTag() const {
		return d->Request.getTag();
	}

	/*!
		\since Visindigo 0.17.0

		返回本次请求使用的描述对象副本。跟随重定向或应用拦截器都不会改写它，
		因此它始终是调用方最初提交的那一份设置。
	*/
	HttpRequest HttpReply::getRequest() const {
		return d->Request;
	}

	/*!
		\since Visindigo 0.17.0

		返回当前已知的响应。响应头尚未到达时其 isValid() 为 \c false。
		请求结束后可通过 getState() 判断它是成功还是失败的结果。
	*/
	HttpResponse HttpReply::getResponse() const {
		return d->Response;
	}

	/*!
		\since Visindigo 0.17.0

		返回句柄当前所处的状态。
	*/
	HttpReply::State HttpReply::getState() const {
		return d->State;
	}

	/*!
		\since Visindigo 0.17.0

		判断请求是否仍在进行中。

		return 请求仍在进行中时返回 true。
	*/
	bool HttpReply::isRunning() const {
		return d->State == State::Queued || d->State == State::Running
			|| d->State == State::ReceivingBody;
	}

	/*!
		\since Visindigo 0.17.0

		中止请求。若请求尚在队列中则直接出队，若已发出则关闭底层传输。

		中止后句柄会依次发出 \l{aborted} 与 \l{finished} 信号，但不会发出 \l{failed}，
		因为"被调用方主动放弃"与"请求失败"是两件事，用同一个信号表达会让错误处理
		逻辑无法区分二者。
	*/
	void HttpReply::abort() {
		if (d->Completed) {
			return;
		}
		d->AbortRequested = true;
		HttpError error;
		error.Type = HttpError::ErrorType::Aborted;
		error.Message = QStringLiteral("Request aborted by caller");
		d->finalizeFailure(error, false);
	}

	/*!
		\since Visindigo 0.17.0

		返回句柄是否会在结束后自动销毁，默认 \c true。

		return 会自动销毁时返回 true。
	*/
	bool HttpReply::isAutoDelete() const {
		return d->AutoDelete;
	}

	/*!
		\since Visindigo 0.17.0

		设置结束后是否自动销毁。

		\note 把默认值改为 \c false 之后，释放句柄的责任就转移到调用方，
		必须自行在适当的时机 delete，否则每次请求都会泄漏一个句柄。

		\a autoDelete 是否在结束后自动销毁。
	*/
	void HttpReply::setAutoDelete(bool autoDelete) {
		d->AutoDelete = autoDelete;
	}

	/*!
		\since Visindigo 0.17.0

		注册请求开始发出的回调，等价于连接 \l started 信号。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::onStarted(QObject* context, std::function<void(quint64 id)> fn) {
		QObject::connect(this, &HttpReply::started, effectiveContext(context, this), fn);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册响应头到达的回调，等价于连接 \l headersReceived 信号。

		这是校验状态码的最佳时机：对下载任务而言，此刻一个字节都还没有写入目标文件，
		发现状态码不对就直接 abort()，不会留下任何残缺文件。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::onHeaders(QObject* context, std::function<void(const HttpResponse&)> fn) {
		QObject::connect(this, &HttpReply::headersReceived, effectiveContext(context, this), fn);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册上传进度回调。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::onUploadProgress(QObject* context, std::function<void(qint64 sent, qint64 total)> fn) {
		QObject::connect(this, &HttpReply::uploadProgress, effectiveContext(context, this), fn);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册下载进度回调。\c total 为 -1 表示服务端未给出长度，此时只能显示已接收字节数。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::onProgress(QObject* context, std::function<void(qint64 received, qint64 total)> fn) {
		QObject::connect(this, &HttpReply::downloadProgress, effectiveContext(context, this), fn);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册即将重试的回调。可用于向用户提示"网络不稳定，正在重试"。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::onRetrying(QObject* context,
		std::function<void(qint32 attempt, qint32 delayMs, const HttpError& lastError)> fn) {
		QObject::connect(this, &HttpReply::retrying, effectiveContext(context, this), fn);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册失败回调，等价于连接 \l failed 信号。主动 abort() 不会触发本回调。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::onError(QObject* context, std::function<void(const HttpError&)> fn) {
		QObject::connect(this, &HttpReply::failed, effectiveContext(context, this), fn);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册被中止的回调。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::onAborted(QObject* context, std::function<void()> fn) {
		QObject::connect(this, &HttpReply::aborted, effectiveContext(context, this), fn);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册结束回调，无论成功、失败还是被中止都会触发。适合放置"关闭进度条"这类
		必须执行的收尾动作。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::onFinally(QObject* context, std::function<void()> fn) {
		QObject::connect(this, &HttpReply::finished, effectiveContext(context, this),
			[fn](const HttpResponse&) { fn(); });
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册成功回调。

		\note 对流式请求而言，本回调表示"流正常结束"，而不是"一次性拿到了全部内容"。
		流中的每个事件应当通过 \l{onFrame} 或 \c{when()} 消费。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数。
	*/
	HttpReply& HttpReply::then(QObject* context, std::function<void(const HttpResponse&)> fn) {
		QObject::connect(this, &HttpReply::succeeded, effectiveContext(context, this), fn);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册原始字节增量回调。处理器返回 \c false 表示本次请求应当立即中止，
		这是流式解析中最常用的提前退出手段——例如已经解析到需要的字段时。

		\note 只有字节未被完整缓存时才逐块交付，即落盘或流式场景。若响应走内存缓存，
		用 \l then 一次性取走即可，逐块回调反而是多余的拷贝。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数，返回 \c false 表示立即中止请求。
	*/
	HttpReply& HttpReply::onRawChunk(QObject* context, std::function<bool(const QByteArray&)> fn) {
		QObject::connect(this, &HttpReply::bodyChunkReceived, effectiveContext(context, this),
			[this, fn](const QByteArray& chunk) {
				if (!fn(chunk)) {
					abort();
				}
			});
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		注册已解码帧的回调，处理器返回 \c false 表示立即中止请求。

		未配置解码器时本回调不会被触发，因此它通常与
		Visindigo::Network::HttpRequest::setStreamDecoder() 配合使用。

		\a context 回调的宿主对象；传 \c nullptr 表示使用句柄自身。
		\a fn 回调函数，返回 \c false 表示立即中止请求。
	*/
	HttpReply& HttpReply::onFrame(QObject* context, std::function<bool(const StreamFrame&)> fn) {
		QObject::connect(this, &HttpReply::frameReceived, effectiveContext(context, this),
			[this, fn](const StreamFrame& frame) {
				if (!fn(frame)) {
					abort();
				}
			});
		return *this;
	}

	HttpReply& HttpReply::then(std::function<void(const HttpResponse&)> fn) {
		return then(nullptr, fn);
	}

	HttpReply& HttpReply::onError(std::function<void(const HttpError&)> fn) {
		return onError(nullptr, fn);
	}

	HttpReply& HttpReply::onFinally(std::function<void()> fn) {
		return onFinally(nullptr, fn);
	}

	HttpReply& HttpReply::onFrame(std::function<bool(const StreamFrame&)> fn) {
		return onFrame(nullptr, fn);
	}

	/*!
		\since Visindigo 0.17.0

		把请求的生命周期绑定到另一个对象上：该对象被销毁时，请求自动中止。

		这与回调糖的 context 参数解决的并不是同一个问题。回调的 context 只能保证
		"对象销毁后不再回调"，但底层连接仍在继续传输，对长连接而言这意味着持续的
		流量与资源占用。本函数会真正把连接关掉，因此流式请求应当调用它。

		\a context 要绑定到的对象；传 \c nullptr 表示不绑定。
	*/
	HttpReply& HttpReply::bindLifecycleTo(QObject* context) {
		if (context != nullptr) {
			QObject::connect(context, &QObject::destroyed, this, [this]() {
				abort();
			});
		}
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		阻塞等待请求结束，返回结果或错误。\a timeoutMs 为 -1 表示不设额外上限。

		\warning 本函数内部运行局部事件循环，在 UI 线程使用会让界面卡住不动，
		并且可能重入其它事件处理逻辑。它只适合在后台线程或初始化阶段使用。
	*/
	HttpResult HttpReply::waitForFinished(qint32 timeoutMs) {
		if (d->State == State::Finished || d->State == State::Aborted) {
			if (d->Response.getError().isError()) {
				return std::unexpected(d->Response.getError());
			}
			return d->Response;
		}
		// 等待期间禁用自动销毁，否则句柄可能在事件循环内被 deleteLater 掉，
		// 让本函数在返回前访问到已释放的对象。
		const bool autoDelete = d->AutoDelete;
		d->AutoDelete = false;

		QEventLoop loop;
		QTimer deadline;
		deadline.setSingleShot(true);
		QObject::connect(&deadline, &QTimer::timeout, &loop, &QEventLoop::quit);
		QObject::connect(this, &HttpReply::finished, &loop, &QEventLoop::quit);
		if (timeoutMs > 0) {
			deadline.start(timeoutMs);
		}
		loop.exec();

		HttpResult result = d->Response.getError().isError()
			? HttpResult(std::unexpected(d->Response.getError()))
			: HttpResult(d->Response);
		if (d->State != State::Finished && d->State != State::Aborted) {
			HttpError error;
			error.Type = HttpError::ErrorType::Timeout;
			error.Message = QStringLiteral("waitForFinished timed out");
			abort();
			result = HttpResult(std::unexpected(error));
		}
		d->AutoDelete = autoDelete;
		if (autoDelete && d->Completed) {
			deleteLater();
		}
		return result;
	}
}
