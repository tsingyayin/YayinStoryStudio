#ifndef Visindigo_Network_private_WebSocketRequest_p_h
#define Visindigo_Network_private_WebSocketRequest_p_h
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include "Network/WebSocketRequest.h"

class QTimer;
class QWebSocket;

namespace Visindigo::Network {
	class WebSocketRequestPrivate {
	public:
		QUrl Url;
		QMap<QString, QString> Headers;
		QString SubProtocol;
		qint32 HeartbeatMs = 0;
		bool AutoReconnect = true;
		qint32 MaxReconnectAttempts = 5;
		qint64 MaxMessageBytes = 0;
		bool VerifyTls = true;
	};

	class WebSocketSessionPrivate {
	public:
		WebSocketSession* Q = nullptr;
		QWebSocket* Socket = nullptr;
		QTimer* HeartbeatTimer = nullptr;
		QTimer* ReconnectTimer = nullptr;
		WebSocketRequest Request;
		QUrl CurrentUrl;
		quint64 Id = 0;
		qint32 ReconnectAttempt = 0;
		bool ManualClose = false;
		bool Started = false;
	public:
		// 建立连接。首次调用与自动重连共用同一条路径。
		void start();
		void scheduleReconnect();
		void teardownSocket();
	};
}
#endif // Visindigo_Network_private_WebSocketRequest_p_h
