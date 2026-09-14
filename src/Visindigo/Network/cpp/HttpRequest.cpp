#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qmap.h>
#include <QtCore/qmimetype.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include <QtCore/qurlquery.h>
#include <QtCore/quuid.h>
#include "Network/HttpRequest.h"
#include "Network/HttpTypes.h"
#include "Network/private/HttpRequest_p.h"
#include "Utility/JsonConfig.h"

namespace {
	// 多部分表单中的字段名与文件名不得含有 CR/LF，否则可以伪造额外的头行。
	// 这里直接剔除控制字符，属于必需的安全处理而非美化。
	QString sanitizeMultipartName(const QString& name) {
		QString result = name;
		result.remove(u'\r');
		result.remove(u'\n');
		result.replace(u'"', u'\'');
		return result;
	}

	// 从 Content-Disposition 一类的头部值中取出指定参数。
	// 只做此类头部实际需要的解析：跳过前导类型标记，再按分号切分参数，
	// 参数值可能带引号，引号内的分号不视作分隔符。
	QString headerParameter(const QString& value, const QString& key, bool* found = nullptr) {
		if (found != nullptr) {
			*found = false;
		}
		qsizetype pos = value.indexOf(u';');
		if (pos < 0) {
			return QString();
		}
		pos += 1;
		while (pos < value.size()) {
			// 跳过空白与分隔符
			while (pos < value.size() && (value.at(pos) == u';' || value.at(pos).isSpace())) {
				pos += 1;
			}
			const qsizetype eq = value.indexOf(u'=', pos);
			if (eq < 0) {
				break;
			}
			const QString name = value.mid(pos, eq - pos).trimmed();
			qsizetype valueStart = eq + 1;
			while (valueStart < value.size() && value.at(valueStart).isSpace()) {
				valueStart += 1;
			}
			QString parsed;
			qsizetype valueEnd = valueStart;
			if (valueStart < value.size() && value.at(valueStart) == u'"') {
				valueStart += 1;
				valueEnd = valueStart;
				while (valueEnd < value.size() && value.at(valueEnd) != u'"') {
					if (value.at(valueEnd) == u'\\' && valueEnd + 1 < value.size()) {
						valueEnd += 1;
					}
					valueEnd += 1;
				}
				parsed = value.mid(valueStart, valueEnd - valueStart);
				parsed.replace(QStringLiteral("\\\""), QStringLiteral("\""));
				parsed.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
				valueEnd += 1;
			}
			else {
				while (valueEnd < value.size() && value.at(valueEnd) != u';') {
					valueEnd += 1;
				}
				parsed = value.mid(valueStart, valueEnd - valueStart).trimmed();
			}
			if (name.compare(key, Qt::CaseInsensitive) == 0) {
				if (found != nullptr) {
					*found = true;
				}
				return parsed;
			}
			pos = valueEnd;
		}
		return QString();
	}

	// 剥掉文件名中的目录成分。服务端完全可以返回 "../../x"，若原样交给调用方
	// 拼接保存路径就会写到目标目录之外，因此剥离动作放在这里统一完成。
	QString stripPathComponents(const QString& name) {
		QString result = name;
		result.replace(u'\\', u'/');
		const qsizetype pos = result.lastIndexOf(u'/');
		if (pos >= 0) {
			result = result.mid(pos + 1);
		}
		if (result == QStringLiteral(".") || result == QStringLiteral("..")) {
			result.clear();
		}
		return result;
	}
}

namespace Visindigo::Network {
	/*!
		\class Visindigo::Network::HttpRequest
		\inheaderfile Network/HttpRequest.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 普通 HTTP(S) 请求的描述对象.

		HttpRequest 是纯值类型（pimpl 实现），可以自由拷贝与传递。它的职责只有一个：
		描述"要发什么样的请求"，不承担任何传输行为。真正发起请求需要把本对象交给
		Visindigo::Network::HttpCenter。

		本类采用链式 setter，每个 setter 均返回自身引用，因此可以这样书写：
		\code
			Visindigo::Network::HttpRequest request;
			request.setUrl("https://example.com/api/v1/items")
			       .setBearerToken(token)
			       .setTag("items")
			       .setRetryPolicy(policy);
		\endcode

		\note 本类不承载任何长连接或事件流专属选项。SSE 请使用
		Visindigo::Network::SSERequest，WebSocket 请使用
		Visindigo::Network::WebSocketRequest。

		\sa Visindigo::Network::HttpResponse
		\sa Visindigo::Network::HttpReply
		\sa Visindigo::Network::HttpCenter
	*/

	/*!
		\enum Visindigo::Network::HttpRequest::Method
		\since Visindigo 0.17.0

		HTTP 请求方法。

		\value Get 读取资源，幂等。
		\value Head 只取响应头，常用于探测资源是否存在或大小，幂等。
		\value Post 提交数据，非幂等。
		\value Put 覆盖式写入，幂等。
		\value Patch 局部修改，非幂等。
		\value Delete 删除，幂等。
		\value Options 查询服务端能力，幂等。
		\value Custom 由调用方自行提供方法名，中心原样发出。此类请求默认不参与重试。
	*/

	/*!
		\enum Visindigo::Network::HttpRequest::Priority
		\since Visindigo 0.17.0

		调度优先级。它只影响请求中心内部队列的出队顺序，对网络层没有任何影响：
		高优先级请求会插到低优先级请求前面被送出，但一旦送出，两者占用的带宽完全相同。

		\value Low 后台任务，例如预取、埋点上报。
		\value Normal 默认值，常规用户可见的操作。
		\value High 用户正在等待的操作，例如点击后立刻需要结果。
		\value Critical 阻塞界面渲染的关键请求，例如必要的配置拉取。
	*/

	/*!
		\enum Visindigo::Network::HttpRequest::RedirectPolicy
		\since Visindigo 0.17.0

		重定向跟随策略。

		\value Never 不跟随，把原始的 3xx 响应原样交给调用方。
		\value Safe 推荐值。安全跟随：阻止 https 降级到 http，并在跨源跳转时
			剥离 Authorization 与 Cookie 等敏感头，避免凭据被转发到第三方主机。
		\value Always 无条件跟随。不推荐，仅在明确知道目标可信时使用。
	*/

	/*!
		\enum Visindigo::Network::HttpRequest::BodySink
		\since Visindigo 0.17.0

		响应消息体的消费方式，也就是"字节最终落到哪里"。这里只描述意图，
		运行时如何交付由 Visindigo::Network::HttpReply 的信号与回调决定，
		两者不重复。

		\value Auto 默认值。响应头给出 Content-Length 且不超过上限时缓存在内存，
			长度未知且累计超出上限时按错误中止，以防止内存被无上限的响应撑爆。
		\value Buffer 强制全部缓存到内存，超出 setMaxBufferBytes() 设定的上限即中止。
		\value ToFile 增量落盘，全程不经过内存缓存，适用于大文件下载。
	*/

	HttpRequest::HttpRequest() {
		d = new HttpRequestPrivate();
	}

	/*!
		\since Visindigo 0.17.0

		以方法与目标地址直接构造请求。
	*/
	HttpRequest::HttpRequest(Method method, const QUrl& url) {
		d = new HttpRequestPrivate();
		d->MethodName = method;
		d->Url = url;
	}

	VICopyable_Impl(HttpRequest);
	VIMoveable_Impl(HttpRequest);

	HttpRequest::~HttpRequest() {
		delete d;
	}

	/*!
		\since Visindigo 0.17.0

		设置请求方法。默认是 \l{Method::Get}。

		需要注意的是：把方法改为非幂等方法（如 POST）后，\l{RetryPolicy} 中
		默认开启的 \c{RetryPolicy::RetryIdempotentOnly} 会导致该请求不再被重试。
		这是刻意设计，避免重复提交造成服务端重复处理。

		\a method 请求方法。
	*/
	HttpRequest& HttpRequest::setMethod(Method method) {
		d->MethodName = method;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置自定义方法名，配合 \l Method::Custom 使用。典型场景是 WebDAV 的
		PROPFIND / MKCOL 这类不在标准枚举内的动词。

		方法名必须是合法的 HTTP token（RFC 7230），即只允许字母、数字与
		! # $ % & ' * + - . ^ _ ` | ~ 这些字符。

		\a verb 自定义方法名。
	*/
	HttpRequest& HttpRequest::setCustomMethod(const QByteArray& verb) {
		d->CustomMethod = verb;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		返回设置的自定义方法名，未设置时为空。
	*/
	QByteArray HttpRequest::getCustomMethod() const {
		return d->CustomMethod;
	}

	/*!
		\since Visindigo 0.17.0

		设置目标地址。若传入的是绝对地址，则中心配置的 BaseUrl 不参与拼接。

		\a url 目标地址。
	*/
	HttpRequest& HttpRequest::setUrl(const QUrl& url) {
		d->Url = url;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置目标地址的字符串重载，内部转为 QUrl。

		\a url 目标地址字符串。
	*/
	HttpRequest& HttpRequest::setUrl(const QString& url) {
		d->Url = QUrl(url);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		以相对路径指定目标，实际发送时会与中心配置的 BaseUrl 合并。

		之所以提供这个入口，是为了让调用方不必到处拼接基础地址，从而避免升级域名或
		切换测试环境时遗漏某个硬编码的完整 URL。

		\a relativePath 相对于中心 BaseUrl 的路径。
	*/
	HttpRequest& HttpRequest::setPath(const QString& relativePath) {
		d->Url = QUrl(relativePath);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		追加一个查询参数。同一个键多次调用会得到多个同名参数（形如
		\c "?tag=a&tag=b"），而不会相互覆盖，这一点与 setHeader() 不同。
	*/
	HttpRequest& HttpRequest::setQuery(const QString& key, const QString& value) {
		d->QueryParams[key].append(value);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		批量追加查询参数，同样是追加而非覆盖。

		\a params 查询参数表。
	*/
	HttpRequest& HttpRequest::setQuery(const QMap<QString, QString>& params) {
		for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
			d->QueryParams[it.key()].append(it.value());
		}
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置单个请求头。同名键会被覆盖。
	*/
	HttpRequest& HttpRequest::setHeader(const QString& key, const QString& value) {
		d->Headers.insert(key, value);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		批量设置请求头。

		\a headers 请求头表。
	*/
	HttpRequest& HttpRequest::setHeader(const QMap<QString, QString>& headers) {
		for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
			d->Headers.insert(it.key(), it.value());
		}
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置 Content-Type 请求头。

		\a mime MIME 类型名，例如 \c "application/json"。
	*/
	HttpRequest& HttpRequest::setContentType(const QString& mime) {
		d->Headers.insert(QStringLiteral("Content-Type"), mime);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		QMimeType 重载，直接采用该 MIME 类型的名称。

		\a mime MIME 类型。
	*/
	HttpRequest& HttpRequest::setContentType(const QMimeType& mime) {
		d->Headers.insert(QStringLiteral("Content-Type"), mime.name());
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置 Accept 请求头，声明期望的响应类型。

		\a mime MIME 类型名。
	*/
	HttpRequest& HttpRequest::setAccept(const QString& mime) {
		d->Headers.insert(QStringLiteral("Accept"), mime);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		QMimeType 重载，直接采用该 MIME 类型的名称。

		\a mime MIME 类型。
	*/
	HttpRequest& HttpRequest::setAccept(const QMimeType& mime) {
		d->Headers.insert(QStringLiteral("Accept"), mime.name());
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置 User-Agent。未设置时使用中心配置的默认值。

		\a userAgent User-Agent 取值。
	*/
	HttpRequest& HttpRequest::setUserAgent(const QString& userAgent) {
		d->Headers.insert(QStringLiteral("User-Agent"), userAgent);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置 Bearer 形式的 Authorization 头。

		若本函数未被调用，且中心通过 setAuthProvider() 配置了凭据提供者，
		则由该提供者按目标地址生成凭据。这样可以让令牌刷新逻辑集中在一处，
		而不必在每个发起请求的地方重复实现。

		\a token 访问令牌。
	*/
	HttpRequest& HttpRequest::setBearerToken(const QString& token) {
		d->Headers.insert(QStringLiteral("Authorization"), QStringLiteral("Bearer ") + token);
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置 Basic 认证的 Authorization 头。

		\note Basic 认证只做 Base64 编码，不含任何加密。若目标地址不是 https，
		凭据在链路上等同于明文，因此中心默认开启的证书校验不应被关闭。

		\a user 用户名。
		\a password 密码。
	*/
	HttpRequest& HttpRequest::setBasicAuth(const QString& user, const QString& password) {
		const QByteArray raw = user.toUtf8() + ':' + password.toUtf8();
		d->Headers.insert(QStringLiteral("Authorization"),
			QStringLiteral("Basic ") + QString::fromLatin1(raw.toBase64()));
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		以 JsonConfig 作为请求体，并自动设置 Content-Type 为 application/json
		（若调用方此前已显式设置过 Content-Type，则保留先前的设置）。
	*/
	HttpRequest& HttpRequest::setJsonBody(const Visindigo::Utility::JsonConfig& json) {
		d->Body = json.toString(QJsonDocument::Compact).toUtf8();
		if (!d->Headers.contains(QStringLiteral("Content-Type"))) {
			d->Headers.insert(QStringLiteral("Content-Type"), QStringLiteral("application/json"));
		}
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		以已序列化好的 JSON 字节作为请求体。不做合法性校验，原样发出。

        本函数也自动设置 Content-Type 为 application/json（若调用方此前已显式设置过 Content-Type，则保留先前的设置）。
		\a rawJson 已序列化的 JSON 字节。
	*/
	HttpRequest& HttpRequest::setJsonBody(const QByteArray& rawJson) {
		d->Body = rawJson;
		if (!d->Headers.contains(QStringLiteral("Content-Type"))) {
			d->Headers.insert(QStringLiteral("Content-Type"), QStringLiteral("application/json"));
		}
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		直接设置原始请求体，不自动附加任何 Content-Type。

		\a body 原始请求体字节。
	*/
	HttpRequest& HttpRequest::setRawBody(const QByteArray& body) {
		d->Body = body;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		以 application/x-www-form-urlencoded 形式提交表单字段。

		\a fields 表单字段表。
	*/
	HttpRequest& HttpRequest::setFormBody(const QMap<QString, QString>& fields) {
		QUrlQuery query;
		for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
			query.addQueryItem(it.key(), it.value());
		}
		d->Body = query.toString(QUrl::FullyEncoded).toUtf8();
		d->Headers.insert(QStringLiteral("Content-Type"),
			QStringLiteral("application/x-www-form-urlencoded"));
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		向多部分表单追加一个普通字段。一旦追加过字段或文件，请求体就会在发送时
		按 multipart/form-data 组装，Content-Type 中会带上自动生成的 boundary。

		\note 字段名中的换行符会被剔除，避免通过名称伪造额外的头行。

		\a name 字段名。
		\a value 字段值。
	*/
	HttpRequest& HttpRequest::addMultipartField(const QString& name, const QString& value) {
		d->MultipartFields.append(qMakePair(name, value));
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		向多部分表单追加一个文件字段。文件在发送时读取，因此若文件在提交请求与
		真正发送之间被删除，请求会以错误结束。

		\a name 字段名。
		\a filePath 待上传文件的路径。
	*/
	HttpRequest& HttpRequest::addMultipartFile(const QString& name, const QString& filePath) {
		d->MultipartFiles.append(qMakePair(name, filePath));
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置响应体的消费方式。详见 \l BodySink 的说明。

		\a sink 响应体消费方式。
	*/
	HttpRequest& HttpRequest::setBodySink(BodySink sink) {
		d->SinkKind = sink;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		指定下载文件的落盘路径，并隐含地把消费方式设为 \l{BodySink::ToFile}。

		落盘采用原子替换语义：数据先写入同目录下的临时文件，全部成功后才覆盖目标，
		因此不会出现"看起来完整但实际残缺"的文件。失败或中止时目标路径保持原样。

		落盘只针对最终响应的消息体。重定向途中的 3xx 响应体不会写入文件，
		非 2xx 响应也不会写入——避免把错误页当成下载结果保存下来。

		\a filePath 落盘目标路径。
	*/
	HttpRequest& HttpRequest::setDownloadFilePath(const QString& filePath) {
		d->DownloadFilePath = filePath;
		d->SinkKind = BodySink::ToFile;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置内存缓存上限，默认 64 MiB。超出上限时请求以
		\l HttpError::ErrorType::ResponseTooLarge 失败。

		这个上限存在的意义是防御：若不加限制，一个恶意的或错误的超长响应
		可以直接把进程的内存吃光。

		\a bytes 内存缓存上限，单位为字节。
	*/
	HttpRequest& HttpRequest::setMaxBufferBytes(qint64 bytes) {
		d->MaxBufferBytes = bytes;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置落盘总字节上限，0 表示不限，默认不限。

		它作用于 \l{BodySink::ToFile}，用于避免下载任务被一个无限增长的服务端响应
		拖垮磁盘。

		\a bytes 落盘总字节上限；0 表示不限。
	*/
	HttpRequest& HttpRequest::setMaxResponseBytes(qint64 bytes) {
		d->MaxResponseBytes = bytes;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		挂载流帧解码器。挂载之后，响应的字节流会按该解码器切分成
		Visindigo::Network::StreamFrame，并经 \l HttpReply::frameReceived 交付。

		对于 NDJSON 可以直接使用 Visindigo::Network::JsonLinesDecoder；
		对于 SSE 通常不必手动设置，\l SSERequest 会自动挂载 SseDecoder。

        请注意，被挂载的decoder其所有权由调用方负责，HttpRequest不会对其进行管理。
		\a decoder 解码器实例，所有权由调用方持有。
	*/
	HttpRequest& HttpRequest::setStreamDecoder(std::shared_ptr<IStreamDecoder> decoder) {
		d->Decoder = decoder;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置单帧上限，默认 16 MiB。解码器缓冲超过该长度仍未见分帧符时，
		视图以 \l HttpError::ErrorType::ParseError 失败。

		它防的是"服务端迟迟不发换行"这一种情况：此时解码器的内部缓冲会无限增长。

		\a bytes 单帧上限，单位为字节。
	*/
	HttpRequest& HttpRequest::setMaxFrameBytes(qint64 bytes) {
		d->MaxFrameBytes = bytes;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置总超时毫秒数，0 表示采用中心配置的默认值。

		\note 总超时对长连接是有害的：流式响应可能在总超时到达时被强行掐断。
		流式请求应当使用 setIdleTimeoutMs() 而非本函数。

		\a ms 总超时毫秒数；0 表示采用中心配置的默认值。
	*/
	HttpRequest& HttpRequest::setTimeoutMs(qint32 ms) {
		d->TimeoutMs = ms;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置空闲超时毫秒数，即两次数据到达之间的最大间隔。大于 0 时为显式值，
		等于 0 时采用中心默认值，小于 0 时关闭空闲超时。

		这个超时比总超时更能表达"连接还活着吗"：只要服务端持续推送数据，
		连接就不会被判定为超时，而一旦长时间没有任何字节到达，就说明链路已经出问题。

		\a ms 空闲超时毫秒数；0 采用中心默认值，小于 0 关闭空闲超时。
	*/
	HttpRequest& HttpRequest::setIdleTimeoutMs(qint32 ms) {
		d->IdleTimeoutMs = ms;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置重试策略。默认策略对幂等方法重试 3 次（含首次），采用带抖动的指数退避。

		\a policy 重试策略。
	*/
	HttpRequest& HttpRequest::setRetryPolicy(const RetryPolicy& policy) {
		d->Retry = policy;
		d->RetryPolicySet = true;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置重定向跟随策略，默认 \l{RedirectPolicy::Safe}。

		\a policy 重定向跟随策略。
	*/
	HttpRequest& HttpRequest::setRedirectPolicy(RedirectPolicy policy) {
		d->RedirectMode = policy;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置最大跟随跳数，默认 5。超出后请求以
		\l HttpError::ErrorType::TooManyRedirects 失败。

		这个上限同时也是一个安全边界：没有它，两个互相指向对方的地址可以构成
		无限跳转，从而把请求永久卡死。

		\a count 最大跟随跳数。
	*/
	HttpRequest& HttpRequest::setFollowRedirectsMax(qint32 count) {
		d->FollowRedirectsMax = count;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置调度优先级，默认 \l{Priority::Normal}。

		\a priority 调度优先级。
	*/
	HttpRequest& HttpRequest::setPriority(Priority priority) {
		d->PriorityLevel = priority;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		给请求打上标签，用于按组批量取消，以及把统计结果归到不同业务名下。

		典型用法是给同一个界面发起的全部请求打同一个标签，界面关闭时调用
		Visindigo::Network::HttpCenter::abortByTag() 一次性取消，避免界面销毁后
		仍在接收回调。

		\a tag 标签文本。
	*/
	HttpRequest& HttpRequest::setTag(const QString& tag) {
		d->Tag = tag;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		为本次请求单独指定代理，覆盖中心配置的全局代理。传入空的 QUrl 表示
		"本请求直连"，即便全局配置了代理。

		\a proxy 代理地址；传空的 QUrl 表示本请求直连。
	*/
	HttpRequest& HttpRequest::setProxyOverride(const QUrl& proxy) {
		d->ProxyOverride = proxy;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		设置是否校验证书，默认 \c true。

		\warning 关闭证书校验等同于放弃中间人防护，链路内容可被任意篡改与窃听。
		仅在连接自签名的本地服务等明确受控的场景下使用，且不应成为默认行为。

		\a verify 是否校验证书。
	*/
	HttpRequest& HttpRequest::setVerifyTls(bool verify) {
		d->VerifyTls = verify;
		return *this;
	}

	/*!
		\since Visindigo 0.17.0

		返回当前设置的请求方法。
	*/
	HttpRequest::Method HttpRequest::getMethod() const {
		return d->MethodName;
	}

	/*!
		\since Visindigo 0.17.0

		返回当前设置的目标地址。若使用的是 setPath()，这里得到的是尚未与
		BaseUrl 合并的相对地址。
	*/
	QUrl HttpRequest::getUrl() const {
		return d->Url;
	}

	/*!
		\since Visindigo 0.17.0

		返回请求标签，未设置时为空字符串。
	*/
	QString HttpRequest::getTag() const {
		return d->Tag;
	}

	/*!
		\since Visindigo 0.17.0

		返回当前的响应体消费方式。
	*/
	HttpRequest::BodySink HttpRequest::getBodySink() const {
		return d->SinkKind;
	}

	/*!
		\since Visindigo 0.17.0

		组装最终请求体。若存在多部分表单字段，则按 multipart/form-data 规则拼接，
		并通过 \a contentTypeOut 回传带 boundary 的 Content-Type；否则直接返回
		已设置的请求体。
	*/
	QByteArray HttpRequestPrivate::buildFinalBody(QByteArray* contentTypeOut) const {
		if (MultipartFields.isEmpty() && MultipartFiles.isEmpty()) {
			return Body;
		}
		const QString boundary = QStringLiteral("----VisindigoBoundary")
			+ QUuid::createUuid().toString(QUuid::WithoutBraces);
		const QByteArray boundaryBytes = boundary.toLatin1();
		QByteArray result;
		for (const auto& field : MultipartFields) {
			result += "--" + boundaryBytes + "\r\n";
			result += "Content-Disposition: form-data; name=\""
				+ sanitizeMultipartName(field.first).toUtf8() + "\"\r\n\r\n";
			result += field.second.toUtf8();
			result += "\r\n";
		}
		for (const auto& file : MultipartFiles) {
			QFile fileHandle(file.second);
			QByteArray fileData;
			if (fileHandle.open(QIODevice::ReadOnly)) {
				fileData = fileHandle.readAll();
				fileHandle.close();
			}
			result += "--" + boundaryBytes + "\r\n";
			result += "Content-Disposition: form-data; name=\""
				+ sanitizeMultipartName(file.first).toUtf8()
				+ "\"; filename=\""
				+ sanitizeMultipartName(QFileInfo(file.second).fileName()).toUtf8()
				+ "\"\r\n";
			result += "Content-Type: application/octet-stream\r\n\r\n";
			result += fileData;
			result += "\r\n";
		}
		result += "--" + boundaryBytes + "--\r\n";
		if (contentTypeOut != nullptr) {
			*contentTypeOut = QByteArrayLiteral("multipart/form-data; boundary=") + boundaryBytes;
		}
		return result;
	}

	/*!
		\since Visindigo 0.17.0

		结合中心的基础地址与已设置的查询参数，得出最终请求地址。
		相对地址会基于 \a baseUrl 解析，已存在的查询参数会被保留。
	*/
	QUrl HttpRequestPrivate::buildFinalUrl(const QUrl& baseUrl) const {
		QUrl finalUrl = Url;
		if (finalUrl.isRelative() && !baseUrl.isEmpty()) {
			finalUrl = baseUrl.resolved(Url);
		}
		if (!QueryParams.isEmpty()) {
			QUrlQuery query(finalUrl);
			for (auto it = QueryParams.constBegin(); it != QueryParams.constEnd(); ++it) {
				for (const QString& value : it.value()) {
					query.addQueryItem(it.key(), value);
				}
			}
			finalUrl.setQuery(query);
		}
		return finalUrl;
	}

	/*!
		\class Visindigo::Network::HttpResponse
		\inheaderfile Network/HttpRequest.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief HTTP 响应描述对象.

		HttpResponse 是值类型，可以自由拷贝与传递。它同时承担两个角色：在请求
		进行中作为"已到达的元数据"载体（由 \l HttpReply::headersReceived 交付），
		在请求结束后作为"最终结果"载体（由 \l HttpReply::finished 交付）。

		消息体在何时可用，取决于请求设置的 \l{HttpRequest::BodySink}：
		\list
		\li \l{HttpRequest::BodySink::Auto}、\l{HttpRequest::BodySink::Buffer}：
			请求结束后可通过 getBody() 取得。
		\li \l{HttpRequest::BodySink::ToFile}：getBody() 始终为空，消息体的载体是文件，
			路径由 getDownloadFilePath() 给出。
		\endlist
		流式请求在结束前消息体也尚未收齐，应通过 \l HttpReply::bodyChunkReceived
		或 \l HttpReply::frameReceived 增量消费。

		\note 头部键名统一以小写形式保存，getHeader() 的查找不区分大小写，
		这与 HTTP 规范一致。

		\sa Visindigo::Network::HttpRequest
		\sa Visindigo::Network::HttpReply
	*/

	HttpResponse::HttpResponse() {
		d = new HttpResponsePrivate();
	}

	VICopyable_Impl(HttpResponse);
	VIMoveable_Impl(HttpResponse);

	HttpResponse::~HttpResponse() {
		delete d;
	}

	/*!
		\since Visindigo 0.17.0

		判断本对象是否已经承载了有效的响应，即响应头是否已经到达。

		它表示的是"是否收到响应头"，而不是"请求是否已经完成"。因此
		\l HttpReply::headersReceived 交付的对象其 isValid() 为 \c true ，
		此时消息体可能还没有任何数据。若想知道请求整体是否结束，应查看
		Visindigo::Network::HttpReply::getState()。

		return 响应头已经到达时返回 true。
	*/
	bool HttpResponse::isValid() const {
		return d->Valid;
	}

	/*!
		\since Visindigo 0.17.0

		返回 HTTP 状态码，例如 200、404。响应头尚未到达时返回 0。
	*/
	qint32 HttpResponse::getStatusCode() const {
		return d->StatusCode;
	}

	/*!
		\since Visindigo 0.17.0

		返回状态行中的原因短语，例如 \c "Not Found"。该字段完全由服务端提供，
		可能为空，也不应与状态码共同作为判断依据。
	*/
	QString HttpResponse::getReasonPhrase() const {
		return d->ReasonPhrase;
	}

	/*!
		\since Visindigo 0.17.0

		判断状态码是否在 2xx 区间，即请求是否被服务端正常受理。

		return 状态码在 2xx 区间时返回 true。
	*/
	bool HttpResponse::isSuccess() const {
		return d->StatusCode >= 200 && d->StatusCode < 300;
	}

	/*!
		\since Visindigo 0.17.0

		按名称取响应头，不区分大小写。不存在时返回空字符串。

		\note HTTP 允许同名头出现多次。本函数只返回其中之一，若确实需要拿到全部
		同名值，请使用 getHeaders() 并自行处理合并规则。

		\a key 头名，不区分大小写。
	*/
	QString HttpResponse::getHeader(const QString& key) const {
		return d->Headers.value(key.toLower());
	}

	/*!
		\since Visindigo 0.17.0

		返回全部响应头。键名已统一为小写。
	*/
	QMap<QString, QString> HttpResponse::getHeaders() const {
		return d->Headers;
	}

	/*!
		\since Visindigo 0.17.0

		返回消息体。仅在 \l HttpRequest::BodySink::Auto 与
		\l HttpRequest::BodySink::Buffer 两种方式下有内容，其余情况返回空字节数组。
	*/
	QByteArray HttpResponse::getBody() const {
		return d->Body;
	}

	/*!
		\since Visindigo 0.17.0

		把消息体按 UTF-8 解码为字符串。

		\note 本函数不解析 Content-Type 中的字符集参数。若服务端返回的是 GBK 等
		非 UTF-8 编码，需要调用方自行取出 getBody() 并转码。
	*/
	QString HttpResponse::getBodyText() const {
		return QString::fromUtf8(d->Body);
	}

	/*!
		\since Visindigo 0.17.0

		把消息体解析为 JSON 对象。解析结果会被缓存，重复调用不会重复解析。

		解析失败时返回空对象，具体原因可通过 getJsonParseError() 获取。
	*/
	Visindigo::Utility::JsonConfig HttpResponse::getJson() const {
		if (!d->JsonParsed) {
			const QJsonParseError error = d->Json.parse(QString::fromUtf8(d->Body));
			d->JsonParseError = error.error == QJsonParseError::NoError
				? QString()
				: error.errorString();
			d->JsonParsed = true;
		}
		return d->Json;
	}

	/*!
		\since Visindigo 0.17.0

		返回 JSON 解析失败的原因，解析成功时为空字符串。
		若尚未调用过 getJson()，本函数会先触发一次解析。
	*/
	QString HttpResponse::getJsonParseError() const {
		if (!d->JsonParsed) {
			getJson();
		}
		return d->JsonParseError;
	}

	/*!
		\since Visindigo 0.17.0

		返回最终响应所在的地址。未跟随重定向时，它与请求地址相同；
		跟随过重定向时，它是最后一跳的地址。
	*/
	QUrl HttpResponse::getFinalUrl() const {
		return d->FinalUrl;
	}

	/*!
		\since Visindigo 0.17.0

		返回重定向过程中依次经过的地址，按发生顺序排列，不含最终地址。
	*/
	QList<QUrl> HttpResponse::getRedirectChain() const {
		return d->RedirectChain;
	}

	/*!
		\since Visindigo 0.17.0

		返回下载文件的实际落盘路径。仅在 \l HttpRequest::BodySink::ToFile 且
		请求成功时有值。
	*/
	QString HttpResponse::getDownloadFilePath() const {
		return d->DownloadFilePath;
	}

	/*!
		\since Visindigo 0.17.0

		从 Content-Disposition 响应头中提取服务端建议的保存文件名。

		解析规则遵循 RFC 6266：优先采用 \c filename* 参数（RFC 5987 编码，
		会被百分号解码并支持 UTF-8 文件名），缺失时退回 \c filename 参数。
		这是因为 \c filename 参数在历史上只能表达 Latin-1，中文名会变成乱码，
		所以服务端同时给出两者时必须以 \c filename* 为准。

		为安全起见，返回值只包含文件名本身，任何目录成分都已被剥除。这样即便
		服务端返回 \c "../../x"，调用方直接用它拼接保存路径也不会写到目标目录之外。

		\note 本函数与实际落盘路径无关——落盘路径由请求侧的
		setDownloadFilePath() 指定。它只服务于"另存为对话框的默认文件名"这类需求。
		未提供可用文件名时返回空字符串，此时调用方可考虑回退为
		Visindigo::Network::HttpResponse::getFinalUrl() 的最后一段路径。
	*/
	QString HttpResponse::getSuggestedFileName() const {
		const QString disposition = d->Headers.value(QStringLiteral("content-disposition"));
		if (disposition.isEmpty()) {
			return QString();
		}
		bool found = false;
		QString name = headerParameter(disposition, QStringLiteral("filename*"), &found);
		if (found && !name.isEmpty()) {
			// RFC 5987 形式为 charset'language'percent-encoded-value
			const qsizetype firstQuote = name.indexOf(u'\'');
			if (firstQuote >= 0) {
				const qsizetype secondQuote = name.indexOf(u'\'', firstQuote + 1);
				if (secondQuote >= 0) {
					name = name.mid(secondQuote + 1);
				}
			}
			name = QUrl::fromPercentEncoding(name.toUtf8());
		}
		else {
			name = headerParameter(disposition, QStringLiteral("filename"), &found);
			if (!found) {
				return QString();
			}
		}
		return stripPathComponents(name);
	}

	/*!
		\since Visindigo 0.17.0

		判断 Content-Disposition 是否声明的 disposition 类型为 \c attachment，
		即服务端明确把响应当作"下载"而非"就地展示"。该头缺失时返回 \c false。

		return 该头声明的类型为 attachment 时返回 true。
	*/
	bool HttpResponse::isAttachment() const {
		const QString disposition = d->Headers.value(QStringLiteral("content-disposition"));
		if (disposition.isEmpty()) {
			return false;
		}
		const qsizetype semicolon = disposition.indexOf(u';');
		const QString type = (semicolon < 0 ? disposition : disposition.left(semicolon)).trimmed();
		return type.compare(QStringLiteral("attachment"), Qt::CaseInsensitive) == 0;
	}

	/*!
		\since Visindigo 0.17.0

		返回已接收的消息体字节数。结合 getTotalBytes() 可以计算下载进度。
        做个简单的除法即可得到百分比。
	*/
	qint64 HttpResponse::getReceivedBytes() const {
		return d->ReceivedBytes;
	}

	/*!
		\since Visindigo 0.17.0

		返回消息体总长度，来自 Content-Length。无法确定时返回 -1，
		这在使用分块传输编码或流式响应时是常见情况，此时只能用已接收字节数
		展示"已下载多少"，无法计算百分比。
	*/
	qint64 HttpResponse::getTotalBytes() const {
		return d->TotalBytes;
	}

	/*!
		\since Visindigo 0.17.0

		返回耗时统计。由于 Qt 的公开接口只暴露总耗时与首字节时刻，其余细分字段
		在没有替换传输后端之前恒为 -1。
	*/
	HttpTimings HttpResponse::getTimings() const {
		return d->Timings;
	}

	/*!
		\since Visindigo 0.17.0

		返回本次请求的错误信息。请求成功时其 \l HttpError::isError() 为 \c false。
	*/
	HttpError HttpResponse::getError() const {
		return d->Error;
	}

	/*!
		\since Visindigo 0.17.0

		返回本次请求的追踪标识，由请求中心分配。它在整个请求生命周期内不变，
		即便发生了重定向或重试也保持同一个值，因此可以用它把散布在多条日志中的
		记录串成一次完整的请求过程。
	*/
	QString HttpResponse::getTraceId() const {
		return d->TraceId;
	}
}
