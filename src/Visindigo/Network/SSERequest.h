#ifndef Visindigo_Network_SSERequest_h
#define Visindigo_Network_SSERequest_h
#include <functional>
#include <memory>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include "Network/HttpRequest.h"
#include "Network/HttpTypes.h"
#include "VICompileMacro.h"
// Forward declarations
class QMimeType;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Network {
	class HttpCenter;
	class HttpCenterPrivate;
	class SSERequestPrivate;
	class SSEReplyPrivate;
}
// Main
namespace Visindigo::Network {
	class SseDecoderPrivate;
	class VisindigoAPI SseDecoder : public IStreamDecoder {
	public:
		SseDecoder();
		virtual ~SseDecoder();
		SseDecoder(const SseDecoder& other) = delete;
		SseDecoder& operator=(const SseDecoder& other) = delete;
	public:
		virtual void reset() override;
		virtual QList<StreamFrame> feed(const QByteArray& chunk) override;
		virtual QList<StreamFrame> finish() override;
		virtual qint64 getBufferedBytes() const override;
		virtual void setMaxFrameBytes(qint64 bytes) override;
		virtual qint64 getMaxFrameBytes() const override;
	private:
		SseDecoderPrivate* d;
	};

	class VisindigoAPI SSERequest {
		friend class HttpCenter;
		friend class HttpCenterPrivate;
	public:
		using Method = HttpRequest::Method;
		using Priority = HttpRequest::Priority;
		using RedirectPolicy = HttpRequest::RedirectPolicy;
	public:
		SSERequest();
		SSERequest(const QUrl& url);
		explicit SSERequest(const HttpRequest& request);
		VICopyable(SSERequest);
		VIMoveable(SSERequest);
		~SSERequest();
	public:
		HttpRequest& getRequest();
		const HttpRequest& getRequest() const;
		SSERequest& setRequest(const HttpRequest& request);
	public:
		// 目标
		SSERequest& setMethod(Method method);
		SSERequest& setCustomMethod(const QByteArray& verb);         // 需为合法 token
		QByteArray getCustomMethod() const;
		SSERequest& setUrl(const QUrl& url);
		SSERequest& setUrl(const QString& url);
		SSERequest& setPath(const QString& relativePath);            // 基于中心 BaseUrl 拼接
		SSERequest& setQuery(const QString& key, const QString& value);
		SSERequest& setQuery(const QMap<QString, QString>& params);
		// 请求头
		SSERequest& setHeader(const QString& key, const QString& value);
		SSERequest& setHeader(const QMap<QString, QString>& headers);
		SSERequest& setContentType(const QString& mime);
		SSERequest& setContentType(const QMimeType& mime);
		SSERequest& setAccept(const QString& mime);
		SSERequest& setAccept(const QMimeType& mime);
		SSERequest& setUserAgent(const QString& userAgent);
		SSERequest& setBearerToken(const QString& token);            // 未设置时由中心 AuthProvider 填充
		SSERequest& setBasicAuth(const QString& user, const QString& password);
		// 请求体
		SSERequest& setJsonBody(const Visindigo::Utility::JsonConfig& json);
		SSERequest& setJsonBody(const QByteArray& rawJson);
		SSERequest& setRawBody(const QByteArray& body);
		SSERequest& setFormBody(const QMap<QString, QString>& fields);
		SSERequest& addMultipartField(const QString& name, const QString& value);
		SSERequest& addMultipartFile(const QString& name, const QString& filePath);
		SSERequest& setStreamDecoder(std::shared_ptr<IStreamDecoder> decoder); // 默认已挂 SseDecoder
		SSERequest& setMaxFrameBytes(qint64 bytes);                  // 单帧上限，默认 1 MiB
		// 行为
		SSERequest& setTimeoutMs(qint32 ms);                         // 长连接勿设总超时，改用 setIdleTimeoutMs
		SSERequest& setIdleTimeoutMs(qint32 ms);                     // 两次数据间隔上限
		SSERequest& setRetryPolicy(const RetryPolicy& policy);
		SSERequest& setRedirectPolicy(RedirectPolicy policy);
		SSERequest& setFollowRedirectsMax(qint32 count);
		SSERequest& setPriority(Priority priority);
		SSERequest& setTag(const QString& tag);
		SSERequest& setProxyOverride(const QUrl& proxy);
		SSERequest& setVerifyTls(bool verify);
	public:
		Method getMethod() const;
		QUrl getUrl() const;
		QString getTag() const;
		// SSE 专属
		SSERequest& setLastEventId(const QString& id);               // 断线续传起点（Last-Event-ID）
		SSERequest& setAutoReconnect(bool enabled, qint32 maxAttempts = 5);
	public:
		QString getLastEventId() const;
		bool isAutoReconnectEnabled() const;
	private:
		SSERequestPrivate* d;
	};

	class VisindigoAPI SSEReply : public HttpReply {
		Q_OBJECT;
		friend class HttpCenter;
		friend class HttpCenterPrivate;
	signals:
		void reconnecting(qint32 attempt, qint32 delayMs);   // 断线自动重连中
	public:
		virtual ~SSEReply();
	public:
		// handler 返回 false 中止流
		SSEReply& when(const QString& eventName, QObject* context, std::function<bool(const StreamFrame&)> handler);
		SSEReply& when(std::function<bool(const StreamFrame&)> predicate, QObject* context,
			std::function<bool(const StreamFrame&)> handler);
		// 同上，但只触发一次。
		SSEReply& whenOnce(const QString& eventName, QObject* context, std::function<bool(const StreamFrame&)> handler);
		SSEReply& whenOnce(std::function<bool(const StreamFrame&)> predicate, QObject* context,
			std::function<bool(const StreamFrame&)> handler);
		// 无 context 重载：仅用于无捕获 lambda
		SSEReply& when(const QString& eventName, std::function<bool(const StreamFrame&)> handler);
	public:
		QString getLastEventId() const;                   // 最近一次服务端给出的 id，可用于续传
	private:
		SSEReply();
		SSEReplyPrivate* d;
	};
}
#endif // Visindigo_Network_SSERequest_h
