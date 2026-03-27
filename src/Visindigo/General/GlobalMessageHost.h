#ifndef Visindigo_General_GlobalMessageHost_H
#define Visindigo_General_GlobalMessageHost_H
#include "../VICompileMacro.h"
#include <QtCore/qobject.h>
namespace Visindigo::General {
	class GlobalMessageHandlerPrivate;
	using MessageID = quint64;
	struct VisindigoAPI GlobalMessageData {
		QString Title;
		QString Content;
		QStringList Actions;
		quint64 ExpireSeconds = -1;
		bool Async = true;
	};

	class VisindigoAPI GlobalMessageHandler {
		virtual void handleMessage(const GlobalMessageData& data) = 0;
	};

	class VisindigoAPI GlobalMessageHost :public QObject{
		Q_OBJECT;
	private:
		GlobalMessageHost();
	signals:
		void messageHandled(MessageID id, const GlobalMessageData& data, const QString& action);
	public:
		static GlobalMessageHost* getInstance();
		~GlobalMessageHost() {};
		MessageID postMessage(const GlobalMessageData& data);
	};
}
#endif // Visindigo_General_GlobalMessageHost_H