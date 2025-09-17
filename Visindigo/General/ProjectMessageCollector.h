#pragma once
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include "../Macro.h"
#include "./ProjectMessageData.h"
// Forward declarations
namespace Visindigo::General {
	class ProjectMessageCollectorPrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI ProjectMessageCollector :public QObject {
		Q_OBJECT;
	signals:
		void messageAdded(const ProjectMessageData& data);
		void messageRemoved(const ProjectMessageData& data);
		void messagesCleared();
	public:
		ProjectMessageCollector();
		~ProjectMessageCollector();
		static ProjectMessageCollector* getInstance();
		void addMessage(const ProjectMessageData& data);
		void removeMessage(const ProjectMessageData& data);
		void clearMessages();
		QList<ProjectMessageData> getMessages() const;
	private:
		ProjectMessageCollectorPrivate* d;
	};
}