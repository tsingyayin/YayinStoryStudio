#ifndef Visindigo_Network_HttpCenter_h
#define Visindigo_Network_HttpCenter_h
#include "Network/HttpRequest.h"
#include "Network/SSERequest.h"
#include "Network/WebSocketRequest.h"
#include "VICompileMacro.h"
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>
#include <functional>
// Forward declarations
class QNetworkAccessManager;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Network {
	class HttpCenterPrivate;
}
// Main
namespace Visindigo::Network {
	// 单例；必须在主线程、QCoreApplication 构造之后创建。
	// 为流式请求设 setTimeoutMs 会提前掉断长连接，应改用 setIdleTimeoutMs。
	class VisindigoAPI HttpCenter : public QObject {
		Q_OBJECT;
		friend class HttpCenterPrivate;
	signals:
		void requestStarted(quint64 id, const HttpRequest& request);
		void requestFinished(quint64 id, const HttpResponse& response);
		void requestFailed(quint64 id, const HttpError& error);
		void onlineStateChanged(bool online);
	public:
		// AuthProvider 仅在请求未显式设置凭据时调用
		using RequestInterceptor = std::function<void(HttpRequest&)>;
		using ResponseInterceptor = std::function<void(HttpResponse&)>;
		using AuthProvider = std::function<QString(const QUrl& url)>;
	public:
		static HttpCenter* getInstance();
	public:
		// 全局配置
		void setDefaultBaseUrl(const QUrl& baseUrl);
		QUrl getDefaultBaseUrl() const;
		void setDefaultUserAgent(const QString& userAgent);
		void setDefaultTimeoutMs(qint32 ms);
		void setDefaultIdleTimeoutMs(qint32 ms);
		void setDefaultRetryPolicy(const RetryPolicy& policy);
		void setMaxConcurrentRequests(qint32 count);
		void setMaxQueuedRequests(qint32 count);
		void setAllowedUrlSchemes(const QStringList& schemes);        // 默认 { "https", "http" }
		void setProxy(const QUrl& proxy);                             // 空 URL = 不使用代理
		void setCookieJarEnabled(bool enabled);
		void setCertificateVerificationEnabled(bool enabled);         // 默认 true
		void setLogSensitiveHeaders(bool enabled);                    // 默认 false：不记录 Authorization/Cookie
		// 中间件
		void setAuthProvider(AuthProvider provider);
		void addRequestInterceptor(RequestInterceptor interceptor);
		void addResponseInterceptor(ResponseInterceptor interceptor);
		void clearInterceptors();
	public:
		// 发起：普通 HTTP
		HttpReply* request(const HttpRequest& request);
		HttpReply* get(const QUrl& url);
		HttpReply* postJson(const QUrl& url, const Visindigo::Utility::JsonConfig& body);
		// 发起：SSE
		SSEReply* request(const SSERequest& request);
		// 发起：WebSocket
		WebSocketSession* openWebSocket(const WebSocketRequest& request);
		// 管控
		void abort(quint64 id);
		void abortByTag(const QString& tag);
		void abortAll();
		void setPaused(bool paused);                                  // 暂停出队，不影响进行中的请求
		bool isPaused() const;
		// 状态（实时）
		qint32 getActiveRequestCount() const;                         // 进行中
		qint32 getQueuedRequestCount() const;                         // 排队中
		bool isOnline() const;
		// 统计（累计）
		quint64 getTotalRequestCount() const;                         // 成功+失败
		quint64 getTotalFailureCount() const;
		quint64 getTotalRetryCount() const;
		quint64 getTotalBytesReceived() const;
		quint64 getTotalBytesSent() const;
		qint64 getAverageElapsedMs() const;                           // 无样本时为 0
		void resetStatistics();
	private:		HttpCenter();		HttpCenterPrivate* d;
	};
}
#endif // Visindigo_Network_HttpCenter_h
