#include "../YSSProject.h"
#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include "../Version.h"
#include "../Log.h"
#include "../../Utility/JsonConfig.h"
#include "../../Utility/FileUtility.h"

namespace Visindigo::General {
	class YSSProjectPrivate {
		friend class YSSProject;
	protected:
		static YSSProject* CurrentProject;
		Visindigo::Utility::JsonConfig* ProjectConfig = nullptr;
		QString ConfigPath;
	protected:
		YSSProjectPrivate() {
			ProjectConfig = new Visindigo::Utility::JsonConfig();
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

	YSSProject* YSSProjectPrivate::CurrentProject = nullptr;

	YSSProject::YSSProject() {
		d = new YSSProjectPrivate();
	}
	YSSProject::~YSSProject() {
		delete d;
	}
	bool YSSProject::loadProject(const QString& configPath) {
		QString config = Visindigo::Utility::FileUtility::readAll(configPath);
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
		Visindigo::Utility::FileUtility::saveAll(d->ConfigPath, config);
		return true;
	}
	bool YSSProject::initProject(const QString& folder, const QString& name) {
		QDir dir(folder);
		if (!Visindigo::Utility::FileUtility::isDirExist(folder)) {
			Visindigo::Utility::FileUtility::createDir(folder);
		}
		else {
			if (!Visindigo::Utility::FileUtility::isDirEmpty(folder)) {
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
	Visindigo::Utility::JsonConfig* YSSProject::getProjectConfig() {
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
		//yDebugF << keys;
		//yDebugF << files;
		return files;
	}
	void YSSProject::addEditorOpenedFile(const QString& filePath) {
		QStringList files = getEditorOpenedFiles();
		if (files.contains(filePath)) {
			return;
		}
		QString relativePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), filePath);
		d->ProjectConfig->setString("Editor.OpenedFiles." + QString::number(d->ProjectConfig->keys("Editor.OpenedFiles").size()), relativePath);
	}
	void YSSProject::removeEditorOpenedFile(const QString& filePath) {
		QStringList files = getEditorOpenedFiles();
		QString relativePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), filePath);
		if (!files.contains(relativePath)) {
			return;
		}
		d->ProjectConfig->remove("Editor.OpenedFiles." + QString::number(files.indexOf(relativePath)));
	}
	void YSSProject::setFocusedFile(const QString& abs_filePath) {
		if (!Visindigo::Utility::FileUtility::isFileExist(abs_filePath)) {
			yError << "File not exist:" << abs_filePath;
			return;
		}
		QString relativePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), abs_filePath);
		d->ProjectConfig->setString("Editor.FocusedFile", relativePath);
	}
	QString YSSProject::getFocusedFile() {
		QString relativePath = d->ProjectConfig->getString("Editor.FocusedFile");
		if (relativePath.isEmpty()) {
			return QString();
		}
		if (relativePath.startsWith("./")) {
			relativePath = getProjectFolder() + "/" + relativePath.mid(2);
		}
		return relativePath;
	}
	QString YSSProject::getFocusedFileName() {
		QString abs_path = getFocusedFile();
		if (abs_path.isEmpty()) {
			return QString();
		}
		QFileInfo info(abs_path);
		return info.fileName();
	}
	void YSSProject::removeAllEditorOpenedFiles() {
		d->ProjectConfig->remove("Editor.OpenedFiles");
	}
	YSSProject* YSSProject::getCurrentProject() {
		return YSSProjectPrivate::CurrentProject;
	}
	void YSSProject::setCurrentProject(YSSProject* project) {
		if (YSSProjectPrivate::CurrentProject != nullptr) {
			YSSProjectPrivate::CurrentProject->saveProject();
			delete YSSProjectPrivate::CurrentProject;
		}
		YSSProjectPrivate::CurrentProject = project;
	}
}