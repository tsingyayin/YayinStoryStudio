#ifndef Visindigo_Network_HttpRequest_h
#define Visindigo_Network_HttpRequest_h
#include <expected>
#include <functional>
#include <memory>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
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
	class HttpReplyPrivate;
	class HttpRequestPrivate;
	class HttpResponsePrivate;
	class JsonLinesDecoderPrivate;
}
// Main
namespace Visindigo::Network {
	// SSE 请改用 SSERequest
	class VisindigoAPI HttpRequest {
		friend class HttpCenter;
		friend class HttpCenterPrivate;
		friend class HttpReplyPrivate;
	public:
		enum class Method {
			Get,
			Head,
			Post,
			Put,
			Patch,
			Delete,
			Options,
			Custom
		};
		enum class Priority {
			Low = 0,
			Normal = 50,
			High = 100,
			Critical = 200
		};
		enum class RedirectPolicy {
			Never,
			Safe,
			Always
		};
		enum class BodySink {
			Auto,
			Buffer,
			ToFile
		};
	public:
		HttpRequest();
		HttpRequest(Method method, const QUrl& url);
		VICopyable(HttpRequest);
		VIMoveable(HttpRequest);
		~HttpRequest();
	public:
		HttpRequest& setMethod(Method method);
		HttpRequest& setCustomMethod(const QByteArray& verb);         // 须为合法 token，否则回退为 GET
		QByteArray getCustomMethod() const;
		HttpRequest& setUrl(const QUrl& url);
		HttpRequest& setUrl(const QString& url);
		HttpRequest& setPath(const QString& relativePath);           // 基于中心 BaseUrl 拼接
		HttpRequest& setQuery(const QString& key, const QString& value);
		HttpRequest& setQuery(const QMap<QString, QString>& params);

		HttpRequest& setHeader(const QString& key, const QString& value);
		HttpRequest& setHeader(const QMap<QString, QString>& headers);
		HttpRequest& setContentType(const QString& mime);
		HttpRequest& setContentType(const QMimeType& mime);
		HttpRequest& setAccept(const QString& mime);
		HttpRequest& setAccept(const QMimeType& mime);
		HttpRequest& setUserAgent(const QString& userAgent);
		HttpRequest& setBearerToken(const QString& token);
		HttpRequest& setBasicAuth(const QString& user, const QString& password);

		HttpRequest& setJsonBody(const Visindigo::Utility::JsonConfig& json);
		HttpRequest& setJsonBody(const QByteArray& rawJson);
		HttpRequest& setRawBody(const QByteArray& body);
		HttpRequest& setFormBody(const QMap<QString, QString>& fields);
		HttpRequest& addMultipartField(const QString& name, const QString& value);
		HttpRequest& addMultipartFile(const QString& name, const QString& filePath);

		HttpRequest& setBodySink(BodySink sink);
		HttpRequest& setDownloadFilePath(const QString& filePath);
		HttpRequest& setMaxBufferBytes(qint64 bytes);
		HttpRequest& setMaxResponseBytes(qint64 bytes);

		HttpRequest& setStreamDecoder(std::shared_ptr<IStreamDecoder> decoder);
		HttpRequest& setMaxFrameBytes(qint64 bytes);

		HttpRequest& setTimeoutMs(qint32 ms);
		HttpRequest& setIdleTimeoutMs(qint32 ms);
		HttpRequest& setRetryPolicy(const RetryPolicy& policy);
		HttpRequest& setRedirectPolicy(RedirectPolicy policy);
		HttpRequest& setFollowRedirectsMax(qint32 count);
		HttpRequest& setPriority(Priority priority);
		HttpRequest& setTag(const QString& tag);
		HttpRequest& setProxyOverride(const QUrl& proxy);
		HttpRequest& setVerifyTls(bool verify);
	public:
		Method getMethod() const;
		QUrl getUrl() const;
		QString getTag() const;
		BodySink getBodySink() const;
	private:
		HttpRequestPrivate* d;
	};

	class VisindigoAPI HttpResponse {
		friend class HttpCenter;
		friend class HttpCenterPrivate;
		friend class HttpReply;
		friend class HttpReplyPrivate;
	public:
		HttpResponse();
		VICopyable(HttpResponse);
		VIMoveable(HttpResponse);
		~HttpResponse();
	public:
		bool isValid() const;                                        // 已收到响应头（非“已完成”）
		qint32 getStatusCode() const;
		QString getReasonPhrase() const;
		bool isSuccess() const;                                      // 2xx
		QString getHeader(const QString& key) const;
		QMap<QString, QString> getHeaders() const;
		QByteArray getBody() const;                                  // 仅 Auto/Buffer 模式可得
		QString getBodyText() const;
		Visindigo::Utility::JsonConfig getJson() const;              // 解析失败返回空对象
		QString getJsonParseError() const;
		QUrl getFinalUrl() const;                                    // 跟随重定向后的地址
		QList<QUrl> getRedirectChain() const;
		QString getDownloadFilePath() const;                         // BodySink::ToFile 的目标
		// filename* 优先（RFC 5987）；已剥目录成分；无则空串
		QString getSuggestedFileName() const;                        // filename* 优先（RFC 5987，已 percent-decode），
		                                                             // 回退 filename；已剥目录成分；无则空串
		bool isAttachment() const;                                   // disposition 类型为 attachment（头缺失时 false）
		qint64 getReceivedBytes() const;
		qint64 getTotalBytes() const;                                // -1 = 未知（分块/流式）
		HttpTimings getTimings() const;
		HttpError getError() const;
		QString getTraceId() const;                                  // 请求追踪 ID，便于日志关联
	private:
		HttpResponsePrivate* d;
	};

	using HttpResult = std::expected<HttpResponse, HttpError>;

	// 默认在结束后自动 deleteLater；需长期持有请先 setAutoDelete(false)
	class VisindigoAPI HttpReply : public QObject {
		Q_OBJECT;
		friend class HttpCenter;
		friend class HttpCenterPrivate;
		friend class HttpReplyPrivate;
	public:
		// 句柄状态机。
		enum class State {
			Queued,
			Running,
			ReceivingBody,
			Finished,
			Aborted
		};
	signals:
		void queued(quint64 id);
		void started(quint64 id);
		void headersReceived(const HttpResponse& response);   // 响应头到达；body 尚未入场，可在此校验状态码后 abort（下载场景）
		void uploadProgress(qint64 sent, qint64 total);
		void downloadProgress(qint64 received, qint64 total);
		void retrying(qint32 attempt, qint32 delayMs, const HttpError& lastError);
		void bodyChunkReceived(const QByteArray& chunk);      // 未被内存缓存的原始增量（ToFile / 流式）
		void frameReceived(const StreamFrame& frame);         // 挂了 IStreamDecoder 时的已解码帧
		void succeeded(const HttpResponse& response);
		void failed(const HttpError& error);
		void finished(const HttpResponse& response);          // 成功/失败均会发出
		void aborted();
	public:
		quint64 getId() const;
		QString getTag() const;
		HttpRequest getRequest() const;
		HttpResponse getResponse() const;                     // 未收到响应头时 isValid() 为 false；完成后看 getState()
		State getState() const;
		bool isRunning() const;
		void abort();
		bool isAutoDelete() const;
		void setAutoDelete(bool autoDelete);
	public:
		// 回调糖，均返回 false 表示中止本次请求
		HttpReply& onStarted(QObject* context, std::function<void(quint64 id)> fn);
		HttpReply& onHeaders(QObject* context, std::function<void(const HttpResponse&)> fn);
		HttpReply& onUploadProgress(QObject* context, std::function<void(qint64 sent, qint64 total)> fn);
		HttpReply& onProgress(QObject* context, std::function<void(qint64 received, qint64 total)> fn);
		HttpReply& onRetrying(QObject* context, std::function<void(qint32 attempt, qint32 delayMs, const HttpError& lastError)> fn);
		HttpReply& onError(QObject* context, std::function<void(const HttpError&)> fn);
		HttpReply& onAborted(QObject* context, std::function<void()> fn);
		HttpReply& onFinally(QObject* context, std::function<void()> fn);
		HttpReply& then(QObject* context, std::function<void(const HttpResponse&)> fn);
		// 返回 false 中止本次请求
		HttpReply& onRawChunk(QObject* context, std::function<bool(const QByteArray&)> fn);
		HttpReply& onFrame(QObject* context, std::function<bool(const StreamFrame&)> fn);
		// 无 context 重载：仅用于无捕获 lambda
		HttpReply& then(std::function<void(const HttpResponse&)> fn);
		HttpReply& onError(std::function<void(const HttpError&)> fn);
		HttpReply& onFinally(std::function<void()> fn);
		HttpReply& onFrame(std::function<bool(const StreamFrame&)> fn);
	public:
		// context 销毁时自动 abort；长响应建议调用
		HttpReply& bindLifecycleTo(QObject* context);
	public:
		// 同步糖：内部运行局部事件循环，请勿在 UI 线程使用
		HttpResult waitForFinished(qint32 timeoutMs = -1);
	protected:
		// 由请求中心创建
		HttpReply();
		virtual ~HttpReply();
	private:
		HttpReplyPrivate* d;
	};

	class VisindigoAPI JsonLinesDecoder : public IStreamDecoder {
	public:
		JsonLinesDecoder();
		virtual ~JsonLinesDecoder();
		JsonLinesDecoder(const JsonLinesDecoder& other) = delete;
		JsonLinesDecoder& operator=(const JsonLinesDecoder& other) = delete;
	public:
		virtual void reset() override;
		virtual QList<StreamFrame> feed(const QByteArray& chunk) override;
		virtual QList<StreamFrame> finish() override;
		virtual qint64 getBufferedBytes() const override;
		virtual void setMaxFrameBytes(qint64 bytes) override;
		virtual qint64 getMaxFrameBytes() const override;
	private:
		JsonLinesDecoderPrivate* d;
	};
}
#endif // Visindigo_Network_HttpRequest_h
