#include "../ProjectMessageCollector.h"
#include <QtCore/qlist>
namespace YSSCore::General {
	class ProjectMessageCollectorPrivate {
		friend class ProjectMessageCollector;
		QList<ProjectMessageData> Messages;
		static ProjectMessageCollector* Instance;
	};
	ProjectMessageCollector* ProjectMessageCollectorPrivate::Instance = nullptr;
	ProjectMessageCollector::ProjectMessageCollector() {
		d = new ProjectMessageCollectorPrivate();
		if (ProjectMessageCollectorPrivate::Instance == nullptr) {
			ProjectMessageCollectorPrivate::Instance = this;
		}
		else {
			delete ProjectMessageCollectorPrivate::Instance;
		}
	}
	ProjectMessageCollector::~ProjectMessageCollector() {
		delete d;
	}
	ProjectMessageCollector* ProjectMessageCollector::getInstance() {
		if (ProjectMessageCollectorPrivate::Instance == nullptr) {
			new ProjectMessageCollector();
		}
		return ProjectMessageCollectorPrivate::Instance;
	}
	void ProjectMessageCollector::addMessage(const ProjectMessageData& data) {
		d->Messages.append(data);
		emit messageAdded(data);
	}
	void ProjectMessageCollector::removeMessage(const ProjectMessageData& data) {
		if (d->Messages.removeOne(data)) {
			emit messageRemoved(data);
		}
	}
	void ProjectMessageCollector::clearMessages() {
		d->Messages.clear();
		emit messagesCleared();
	}
	QList<ProjectMessageData> ProjectMessageCollector::getMessages() const {
		return d->Messages;
	}
}