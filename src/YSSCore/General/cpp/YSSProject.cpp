#include "../YSSProject.h"
#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <General/Version.h>
#include <General/Log.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
#include "General/YSSLogger.h"
#include <General/Plugin.h>
namespace YSSCore::General {
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
	YSSProject::LoadProjectResult YSSProject::loadProject(const QString& configPath) {
		QString config = Visindigo::Utility::FileUtility::readAll(configPath);
		d->ConfigPath = configPath;
		bool ok = d->ProjectConfig->parse(config).error == QJsonParseError::NoError;
		if (not ok) {
			return LoadProjectResult::ParseError;
		}
		QStringList crucialKeys = {
			"Project.Name",
			"Project.CreateTime",
			"Project.LastModifyTime",
			"Project.Version",
			"Project.DebugServerID",
			"Project.Description",
			"Project.IconPath"
		};
		for (const QString& key : crucialKeys) {
			if (!d->ProjectConfig->contains(key)) {
				yErrorF << "Project config missing crucial key:" << key;
				return LoadProjectResult::InvalidConfig;
			}
		}
		return LoadProjectResult::Success;
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
				yErrorF << "Project folder is not empty:" << folder;
				return false;
			}
		}
		d->ConfigPath = folder + "/project.yssp";
		d->ProjectConfig->setString("Project.Name", name);
		d->ProjectConfig->setString("Project.Description", "");
		d->ProjectConfig->setString("Project.IconPath", "");
		d->ProjectConfig->setString("Project.Version", "0.1");
		d->ProjectConfig->setString("Project.DebugServerID", "");
		d->ProjectConfig->setString("Project.CreateTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		d->ProjectConfig->setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
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
	void YSSProject::setProjectName(const QString& name) {
		d->ProjectConfig->setString("Project.Name", name);
	}
	void YSSProject::setProjectDescription(const QString& description) {
		d->ProjectConfig->setString("Project.Description", description);
	}
	void YSSProject::setProjectIconPath(const QString& iconPath) {
		d->ProjectConfig->setString("Project.IconPath", iconPath);
	}
	QDateTime YSSProject::getProjectCreateTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.CreateTime"), "yyyy-MM-dd hh:mm:ss");
	}
	QDateTime YSSProject::getProjectLastModifyTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.LastModifyTime"), "yyyy-MM-dd hh:mm:ss");
	}
	QString YSSProject::getProjectDebugServerID() {
		return d->ProjectConfig->getString("Project.DebugServerID");
	}
	void YSSProject::setProjectDebugServerID(const QString& id) {
		d->ProjectConfig->setString("Project.DebugServerID", id);
	}
	Visindigo::General::Version YSSProject::getProjectVersion() {
		return Visindigo::General::Version(d->ProjectConfig->getString("Project.Version"));
	}
	
	Visindigo::Utility::JsonConfig YSSProject::getProjectConfigForPlugin(Visindigo::General::Plugin* plugin) {
		QString key = "MetaData." + plugin->getPluginID();
		return d->ProjectConfig->getObject(key);
	}

	Visindigo::Utility::JsonConfig YSSProject::getProjectConfigForPlugin(const QString& pluginID) {
		QString key = "MetaData." + pluginID;
		return d->ProjectConfig->getObject(key);
	}

	void YSSProject::saveProjectConfigForPlugin(Visindigo::General::Plugin* plugin, const Visindigo::Utility::JsonConfig& config) {
		QString key = "MetaData." + plugin->getPluginID();
		d->ProjectConfig->setObject(key, config);
	}

	void YSSProject::saveProjectConfigForPlugin(const QString& pluginID, const Visindigo::Utility::JsonConfig& config) {
		QString key = "MetaData." + pluginID;
		d->ProjectConfig->setObject(key, config);
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
			yErrorF << "File not exist:" << abs_filePath;
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