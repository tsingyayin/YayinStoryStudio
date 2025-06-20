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
	bool YSSProject::initProject(const QString& folder, const QString& name) {
		QDir dir(folder);
		if (!YSSCore::Utility::FileUtility::isDirExist(folder)) {
			YSSCore::Utility::FileUtility::createDir(folder);
		}
		else {
			if (!YSSCore::Utility::FileUtility::isDirEmpty(folder)) {
				yError << "Project folder is not empty:" << folder;
				return false;
			}
		}
		d->ConfigPath = folder + "/project.yssp";
		d->ProjectConfig->setString("Project.Name", name);
		d->ProjectConfig->setString("Project.Description", "");
		d->ProjectConfig->setString("Project.DebugServerID", "");
		d->ProjectConfig->setString("Project.IconPath", "");
		d->ProjectConfig->setString("Project.CreateTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		refreshLastModifyTime();
		saveProject();
		return true;
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
	QStringList YSSProject::getEditorOpenedFiles() {
		QStringList files;
		QStringList keys = d->ProjectConfig->keys("Editor.OpenedFiles");
		for (const QString& key : keys) {
			QString filePath = d->ProjectConfig->getString("Editor.OpenedFiles." + key);
			if (filePath.startsWith("./")) {
				filePath = getProjectFolder() + "/" + filePath.mid(2);
			}
			files.append(filePath);
		}
		return files;
	}
	void YSSProject::addEditorOpenedFile(const QString& filePath) {
		QStringList files = getEditorOpenedFiles();
		if (files.contains(filePath)) {
			yMessage << "File already opened in editor:" << filePath;
			return;
		}
		QString relativePath = YSSCore::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), filePath);
		d->ProjectConfig->setString("Editor.OpenedFiles." + QString::number(d->ProjectConfig->keys("Editor.OpenedFiles").size()), relativePath);
	}
	void YSSProject::removeEditorOpenedFile(const QString& filePath) {
		QStringList files = getEditorOpenedFiles();
		QString relativePath = YSSCore::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), filePath);
		if (!files.contains(relativePath)) {
			yMessage << "File not opened in editor:" << relativePath;
			return;
		}
		d->ProjectConfig->remove("Editor.OpenedFiles." + QString::number(files.indexOf(relativePath)));
	}
	void YSSProject::removeAllEditorOpenedFiles() {
		d->ProjectConfig->remove("Editor.OpenedFiles");
	}
} 