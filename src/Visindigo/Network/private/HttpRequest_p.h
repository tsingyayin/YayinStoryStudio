#ifndef Visindigo_Network_private_HttpRequest_p_h
#define Visindigo_Network_private_HttpRequest_p_h
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>
#include <memory>
#include "Network/HttpRequest.h"
#include "Network/HttpTypes.h"
#include "Utility/JsonConfig.h"
namespace Visindigo::Network {
	class HttpRequestPrivate {
		friend class HttpRequest;
		friend class HttpCenter;
		friend class HttpCenterPrivate;
	public:
		// 生成最终请求体，并在需要时给出 Content-Type（多部分表单需临时 boundary）。
		// 由中心在实际发送前调用，因此每次重试都会得到新的 boundary。
		QByteArray buildFinalBody(QByteArray* contentTypeOut) const;
		// 结合中心的 BaseUrl 与查询参数得到最终 URL。
		QUrl buildFinalUrl(const QUrl& baseUrl) const;
	public:
		HttpRequest::Method MethodName = HttpRequest::Method::Get;
		QByteArray CustomMethod;
		QUrl Url;
		QMap<QString, QStringList> QueryParams;
		QMap<QString, QString> Headers;
		QByteArray Body;
		QList<QPair<QString, QString>> MultipartFields;
		QList<QPair<QString, QString>> MultipartFiles;
		HttpRequest::BodySink SinkKind = HttpRequest::BodySink::Auto;
		QString DownloadFilePath;
		qint64 MaxBufferBytes = 64 * 1024 * 1024;
		qint64 MaxResponseBytes = 0;
		std::shared_ptr<IStreamDecoder> Decoder = nullptr;
		qint64 MaxFrameBytes = 16 * 1024 * 1024;
		qint32 TimeoutMs = 0;
		qint32 IdleTimeoutMs = 0;
		RetryPolicy Retry;
		bool RetryPolicySet = false;            // 调用方是否显式设置过重试策略
		HttpRequest::RedirectPolicy RedirectMode = HttpRequest::RedirectPolicy::Safe;
		qint32 FollowRedirectsMax = 5;
		HttpRequest::Priority PriorityLevel = HttpRequest::Priority::Normal;
		QString Tag;
		QUrl ProxyOverride;
		bool VerifyTls = true;
	};

	class HttpResponsePrivate {
		friend class HttpResponse;
		friend class HttpReply;
		friend class HttpReplyPrivate;
		friend class HttpCenter;
		friend class HttpCenterPrivate;
	public:
		bool Valid = false;
		qint32 StatusCode = 0;
		QString ReasonPhrase;
		QMap<QString, QString> Headers;
		QByteArray Body;
		Visindigo::Utility::JsonConfig Json;
		QString JsonParseError;
		bool JsonParsed = false;
		QUrl FinalUrl;
		QList<QUrl> RedirectChain;
		QString DownloadFilePath;
		qint64 ReceivedBytes = 0;
		qint64 TotalBytes = -1;
		HttpTimings Timings;
		HttpError Error;
		QString TraceId;
	};
}
#endif // Visindigo_Network_private_HttpRequest_p_h
