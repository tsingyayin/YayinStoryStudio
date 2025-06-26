#pragma once
#include "../Macro.h"
#include "./ProjectMessageData.h"
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
namespace YSSCore::General {
	class ProjectMessageCollectorPrivate;
	class YSSCoreAPI ProjectMessageCollector :public QObject {
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