#ifndef Visindigo_Network_private_HttpCenter_p_h
#define Visindigo_Network_private_HttpCenter_p_h
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qnetworkproxy.h>
#include "Network/HttpCenter.h"
#include "Network/HttpRequest.h"
#include "Network/HttpTypes.h"

class QNetworkAccessManager;
class QNetworkInformation;
class QWebSocket;

namespace Visindigo::Network {
	class HttpReply;
	class HttpCenterPrivate;

	// 每个请求都可以单独指定代理，但 Qt 只在 QNetworkAccessManager 级别提供
	// setProxy()，没有“随请求携带代理”的通道，因此这里用代理工厂按主机名查表：
	// 中心在发送前登记、传输结束后注销。注意 QNetworkProxyFactory::queryProxy()
	// 只给出主机名而不含端口，所以同一个主机在同一时刻只能对应一个代理覆盖，
	// 这是本实现已知且可接受的限制。
	class VisindigoProxyFactory : public QNetworkProxyFactory {
	public:
		QHash<QString, QNetworkProxy> Overrides;
		QNetworkProxy Fallback;
		bool HasFallback = false;
	public:
		virtual QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery& query) override;
	};

	class HttpCenterPrivate {
	public:
		// 队列与调度
		void enqueue(HttpReply* reply);
		void release(HttpReply* reply);
		void pump();
		// 配置查询（请求级设置优先于中心默认值）
		bool isSchemeAllowed(const QUrl& url) const;
		qint32 timeoutFor(const HttpRequest& request) const;
		qint32 idleTimeoutFor(const HttpRequest& request) const;
		RetryPolicy retryFor(const HttpRequest& request) const;
		QString userAgentFor(const HttpRequest& request) const;
		// 句柄工厂：SSE 请求会得到 SSEReply，其余得到 HttpReply
		HttpReply* createReply(const HttpRequest& request, bool sse);
		// 统计
		void statStarted(const HttpRequest& request);
		void statFinished(const HttpResponse& response, bool failed);
		void resetStats();
	public:
		HttpCenter* Q = nullptr;
		QNetworkAccessManager* NAM = nullptr;
		QNetworkInformation* NetInfo = nullptr;
		// 以指针保存且不释放：QNetworkAccessManager::setProxyFactory() 在
		// 不同 Qt 版本上对是否接管所有权并不一致，故意泄漏一份来避开这个歧义。
		// 中心是进程内单例，该对象只会创建一次。
		VisindigoProxyFactory* ProxyFactory = nullptr;
		QUrl DefaultBaseUrl;
		QString DefaultUserAgent;
		qint32 DefaultTimeoutMs = 30000;
		qint32 DefaultIdleTimeoutMs = 60000;
		RetryPolicy DefaultRetryPolicy;
		qint32 MaxConcurrentRequests = 6;
		qint32 MaxQueuedRequests = 128;
		QStringList AllowedUrlSchemes = { QStringLiteral("https"), QStringLiteral("http") };
		QUrl Proxy;
		bool CookieJarEnabled = false;
		bool CertificateVerificationEnabled = true;
		bool LogSensitiveHeaders = false;
		HttpCenter::AuthProvider AuthProviderHook;
		QList<HttpCenter::RequestInterceptor> RequestInterceptors;
		QList<HttpCenter::ResponseInterceptor> ResponseInterceptors;
		QList<HttpReply*> Queue;
		QHash<quint64, HttpReply*> Active;
		quint64 NextId = 1;
		bool Paused = false;
		bool Pumping = false;
		bool Online = true;
		// 累计统计
		quint64 StatTotalRequests = 0;
		quint64 StatTotalFailures = 0;
		quint64 StatTotalRetries = 0;
		quint64 StatTotalBytesReceived = 0;
		quint64 StatTotalBytesSent = 0;
		qint64 StatTotalElapsedMs = 0;
		quint64 StatSamplesWithElapsed = 0;
	};
}
#endif // Visindigo_Network_private_HttpCenter_p_h
