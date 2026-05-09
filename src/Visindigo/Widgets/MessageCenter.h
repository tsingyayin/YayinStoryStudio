#ifndef Visindigo_Widgets_MessageCenter_h
#define Visindigo_Widgets_MessageCenter_h
#include "VICompileMacro.h"
#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <General/Logger.h>
namespace Visindigo::Widgets {
	class MessageHandler;
	class MessageCenterPrivate;
	class VisindigoAPI MessageCenter :public QObject{
		Q_OBJECT;
	signals:
		void messageHandled(qint32 handleID, qint32 handleRtn);
	private:
		MessageCenter();
	public:
		~MessageCenter();
		static MessageCenter* getInstance();
		void setMessageHandler(MessageHandler* handler);
		MessageHandler* getHandler() const;
		qint32 show(General::Logger::Level type, const QString& content, QMap<QString, qint32> buttons = {}, qint32 timeoutMS = 30000);
		qint32 info(const QString& content, QMap<QString, qint32> buttons = {}, qint32 timeoutMS = 30000);
		qint32 debug(const QString& content, QMap<QString, qint32> buttons = {}, qint32 timeoutMS = 30000);
		qint32 success(const QString& content, QMap<QString, qint32> buttons = {}, qint32 timeoutMS = 30000);
		qint32 warning(const QString& content, QMap<QString, qint32> buttons = {}, qint32 timeoutMS = 30000);
		qint32 error(const QString& content, QMap<QString, qint32> buttons = {}, qint32 timeoutMS = 30000);
		void listenLogger(const QString& loggerID, General::Logger::Level threshold = General::Logger::Level::Message);
		void unlistenLogger(const QString& loggerID);
		QStringList getListeningLoggers() const;
	private:
		MessageCenterPrivate* d;
	};

	class VisindigoAPI MessageHandler {
	public:
		void onMessageRequest(qint32 messageID, General::Logger::Level type, const QString& content,
			QMap<QString, qint32> buttons, qint32 timeoutMSll);
		void onMessageHandled(qint32 messageID, qint32 buttonID);
	};
}
#endif // Visindigo_Widgets_MessageCenter_h
