#include "../YSSProject.h"
#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include "../Version.h"
#include "../Log.h"
#include "../../Utility/JsonConfig.h"
#include "../../Utility/FileUtility.h"

namespace YSSCore::General {
	class YSSProjectPrivate {
		friend class YSSProject;
	protected:
		YSSCore::Utility::JsonConfig* ProjectConfig = nullptr;
		QString ConfigPath;
	protected:
		YSSProjectPrivate() {
			ProjectConfig = new YSSCore::Utility::JsonConfig();
		}
		~YSSProjectPrivate() {
			if (ProjectConfig != nullptr) {
				delete ProjectConfig;
			}
		}
		void updateLastModifyTime() {
			ProjectConfig->setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		}
	};
	YSSProject::YSSProject() {
		d = new YSSProjectPrivate();
	}
	YSSProject::~YSSProject() {
		delete d;
	}
	bool YSSProject::loadProject(const QString& configPath) {
		QString config = YSSCore::Utility::FileUtility::readAll(configPath);
		d->ConfigPath = configPath;
		return d->ProjectConfig->parse(config).error == QJsonParseError::NoError;
	}
	bool YSSProject::saveProject(const QString& configPath) {
		d->updateLastModifyTime();
		if (configPath.isEmpty()) {
			if (d->ConfigPath.isEmpty()) {
				return false;
			}
		}
		else {
			d->ConfigPath = configPath;
		}
		QString config = d->ProjectConfig->toString();
		YSSCore::Utility::FileUtility::saveAll(d->ConfigPath, config);
		return true;
	}
	void YSSProject::initProject(const QString& folder, const QString& name) {
		d->ConfigPath = folder + "/project.yssp";
		d->ProjectConfig->setString("Project.Name", name);
		d->ProjectConfig->setString("Project.Description", "");
		d->ProjectConfig->setString("Project.DebugServerID", "");
		d->ProjectConfig->setString("Project.IconPath", "");
		d->ProjectConfig->setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		refreshLastModifyTime();
	}
	QString YSSProject::getProjectName() {
		return d->ProjectConfig->getString("Project.Name");
	}
	QString YSSProject::getProjectDescription() {
		return d->ProjectConfig->getString("Project.Description");
	}
	QString YSSProject::getProjectDebugServerID() {
		return d->ProjectConfig->getString("Project.DebugServerID");
	}
	QString YSSProject::getProjectFolder() {
		QFileInfo info(d->ConfigPath);
		return info.absolutePath();
	}
	QString YSSProject::getProjectPath() {
		return d->ConfigPath;
	}
	QString YSSProject::getProjectIconPath() {
		return d->ProjectConfig->getString("Project.IconPath");
	}
	QDateTime YSSProject::getProjectCreateTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.CreateTime"), "yyyy-MM-dd hh:mm:ss");
	}
	QDateTime YSSProject::getProjectLastModifyTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.LastModifyTime"), "yyyy-MM-dd hh:mm:ss");
	}
	Version YSSProject::getProjectVersion() {
		return Version(d->ProjectConfig->getString("Project.Version"));
	}
	YSSCore::Utility::JsonConfig* YSSProject::getProjectConfig() {
		return d->ProjectConfig;
	}
	void YSSProject::refreshLastModifyTime() {
		d->ProjectConfig->setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	}
} 