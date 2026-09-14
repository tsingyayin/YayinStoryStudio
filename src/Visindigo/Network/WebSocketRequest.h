#ifndef Visindigo_Network_WebSocketRequest_h
#define Visindigo_Network_WebSocketRequest_h
#include <QtCore/qbytearray.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include "Network/HttpTypes.h"
#include "VICompileMacro.h"
// Forward declarations
namespace Visindigo::Network {
	class HttpCenter;
	class HttpCenterPrivate;
	class WebSocketRequestPrivate;
	class WebSocketSessionPrivate;
}
// Main
namespace Visindigo::Network {
	class VisindigoAPI WebSocketRequest {
		friend class HttpCenter;
		friend class HttpCenterPrivate;
		friend class WebSocketSessionPrivate;
	public:
		WebSocketRequest();
		WebSocketRequest(const QUrl& url);
		VICopyable(WebSocketRequest);
		VIMoveable(WebSocketRequest);
		~WebSocketRequest();
	public:
		WebSocketRequest& setUrl(const QUrl& url);
		WebSocketRequest& setHeader(const QString& key, const QString& value);
		WebSocketRequest& setSubProtocol(const QString& protocol);
		WebSocketRequest& setHeartbeatMs(qint32 ms);                     // 0 = 不发送心跳
		WebSocketRequest& setAutoReconnect(bool enabled, qint32 maxAttempts = 5);
		WebSocketRequest& setMaxMessageBytes(qint64 bytes);              // 0 = 不限
		WebSocketRequest& setVerifyTls(bool verify);                     // 默认 true
	private:
		WebSocketRequestPrivate* d;
	};

	class VisindigoAPI WebSocketSession : public QObject {
		Q_OBJECT;
		friend class HttpCenter;
		friend class HttpCenterPrivate;
	signals:
		void connected();
		void textMessageReceived(const QString& message);
		void binaryMessageReceived(const QByteArray& message);
		void pingReceived(const QByteArray& payload);
		void reconnecting(qint32 attempt, qint32 delayMs);
		void disconnected(quint16 closeCode, const QString& reason);
		void errorOccurred(const HttpError& error);
	public:
		quint64 getId() const;
		bool isConnected() const;
		void sendText(const QString& text);
		void sendBinary(const QByteArray& data);
		void sendPing(const QByteArray& payload = QByteArray());
		void close(quint16 code = 1000, const QString& reason = QString());
		void abort();
	public:
		// context 销毁时自动关闭会话
		WebSocketSession& bindLifecycleTo(QObject* context);
	private:
		// 会话一律由请求中心创建
		WebSocketSession();
		virtual ~WebSocketSession();
		WebSocketSessionPrivate* d;
	};
}
#endif // Visindigo_Network_WebSocketRequest_h
