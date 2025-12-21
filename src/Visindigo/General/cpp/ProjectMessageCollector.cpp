#include "../ProjectMessageCollector.h"
#include <QtCore/qlist>
namespace Visindigo::General {
	class ProjectMessageCollectorPrivate {
		friend class ProjectMessageCollector;
		QList<ProjectMessageData> Messages;
		static ProjectMessageCollector* Instance;
	};
	ProjectMessageCollector* ProjectMessageCollectorPrivate::Instance = nullptr;

	/*!
		\class Visindigo::General::ProjectMessageCollector
		\brief 项目消息收集器单例类。
		\inmodule Visindigo
		
		\warning 此类没有完全开发，也没有更好的优化方案，随时可能被废弃或替换。
	*/
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