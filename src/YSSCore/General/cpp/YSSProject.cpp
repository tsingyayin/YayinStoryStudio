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

	/*!
		\class YSSCore::General::YSSProject
		\brief 这个类代表YSS项目
		\since YSS 0.13.0
		\inmodule YSSCore

		YSSProject类代表YSS项目。
		
		值得注意的是，这个类目前是个非QObject类，它是作为project.yssp文件的API
		操作以及读写使用，不在程序中充当一些具体功能的提供者。
		
		譬如，如果调用此类的setFocusedFile函数，它只会修改对应yssp文件中记录的数据，
		不会真的使YSS编辑器变更当前打开的文件。这个需求需要用户调用FileServerManager
		的changeFocusedFile信号进行。稍后，YSS会根据是否实际变更了打开的文件来读写YSSP。

		YSSProject不是单例类，你可以根据需要在程序各个地方创建
		任意多的YSSProject示例，但YSSProject语义上只允许一个YSSProject作为
		“当前项目”，你可以通过getCurrentProject和setCurrentProject读写它。当前项目
		可以为nullptr，代表当前没有打开任何项目。

		要从本类直接初始化一个YSS项目，请参见initProject函数的说明。
	*/

	/*!
		\enum YSSCore::General::YSSProject::LoadProjectResult
		\since YSS 0.13.0
		\value Unknown 未知的加载结果
		\value Success 加载成功
		\value ParseError 项目的JSON格式有误
		\value InvalidConfig 项目的JSON格式无误，但配置无效
	*/

	/*!
		\since YSS 0.13.0
		构造函数，创建一个空的项目
	*/
	YSSProject::YSSProject() {
		d = new YSSProjectPrivate();
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	YSSProject::~YSSProject() {
		if (YSSProjectPrivate::CurrentProject == this) {
			YSSProjectPrivate::CurrentProject = nullptr;
		}
		delete d;
	}

	/*!
		\since YSS 0.13.0
		从\a configPath 加载YSS项目
	*/
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

	/*!
		\since YSS 0.13.0
		获取当前项目的配置文件路径。
		如果项目加载时是有效的，那这个返回值就等同于设置值。
		即，它是绝对路径还是相对路径取决于设置时的原样。

		\warning 这个函数从0.17开始弃用，因为功能与getProjectPath()重复。
	*/
	QString YSSProject::getProjectConfigPath() {
		return d->ConfigPath;
	}

	/*!
		\since YSS 0.13.0
		保存项目。
		\a configPath 默认为空，代表在加载位置原地保存。如果configPath
		为其他值，就相当于另存为。
		return 是否保存成功
	*/
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

	/*!
		\since YSS 0.13.0
		在\a folder指定的文件夹初始化项目，名称为\a name

		如果\a folder不存在，那么会自动创建沿途所有不存在的文件夹。

		如果\a folder不为空，会创建失败。

		只要可以创建，这函数执行之后，configPath就被自动设置为指定的位置。

		如果要通过YSSProject初始化YSS项目，一般推荐首先调用此函数以建立项目文件夹，
		稍后再进行其他初始化操作。
	*/
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
		d->ProjectConfig->setString("Project.Author", "");
		d->ProjectConfig->setString("Project.DebugServerID", "");
		d->ProjectConfig->setString("Project.CreateTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		refreshLastModifyTime();
		saveProject();
		return true;
	}

	/*!
		\since YSS 0.13.0
		获得项目名称
	*/
	QString YSSProject::getProjectName() {
		return d->ProjectConfig->getString("Project.Name");
	}

	/*!
		\since YSS 0.13.0
		获得项目描述
	*/
	QString YSSProject::getProjectDescription() {
		return d->ProjectConfig->getString("Project.Description");
	}

	/*!
		\since YSS 0.13.0
		获取当前项目的文件夹路径。

		与getProjectPath有所不同的是，这个函数固定按
		绝对路径返回。
	*/
	QString YSSProject::getProjectFolder() {
		QFileInfo info(d->ConfigPath);
		return info.absolutePath();
	}

	/*!
		\since YSS 0.13.0
		获取当前项目的配置文件路径。

		如果项目加载时是有效的，那这个返回值就等同于设置值。
		即，它是绝对路径还是相对路径取决于设置时的原样。
	*/
	QString YSSProject::getProjectPath() {
		return d->ConfigPath;
	}

	/*!
		\since YSS 0.13.0
		获取当前项目的图标路径。

		这个图标路径忠实返回项目文件设置的值，不管他
		是否真的有效，以及是什么格式的路径。
	*/
	QString YSSProject::getProjectIconPath() {
		return d->ProjectConfig->getString("Project.IconPath");
	}

	/*!
		\since YSS 0.13.0
		获取项目作者设置。
	*/
	QString YSSProject::getProjectAuthor() {
		return d->ProjectConfig->getString("Project.Author");
	}

	/*!
		\since YSS 0.13.0
		设置项目名称为/a name
	*/
	void YSSProject::setProjectName(const QString& name) {
		d->ProjectConfig->setString("Project.Name", name);
	}

	/*!
		\since YSS 0.13.0
		设置项目描述为\a description
	*/
	void YSSProject::setProjectDescription(const QString& description) {
		d->ProjectConfig->setString("Project.Description", description);
	}

	/*!
		\since YSS 0.13.0
		设置项目图标路径为\a iconPath
	*/
	void YSSProject::setProjectIconPath(const QString& iconPath) {
		d->ProjectConfig->setString("Project.IconPath", iconPath);
	}

	/*!
		\since YSS 0.13.0
		设置项目作者为\a author
	*/
	void YSSProject::setProjectAuthor(const QString& author) {
		d->ProjectConfig->setString("Project.Author", author);
	}

	/*!
		\since YSS 0.13.0
		获取项目创建时间
	*/
	QDateTime YSSProject::getProjectCreateTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.CreateTime"), "yyyy-MM-dd hh:mm:ss");
	}

	/*!
		\since YSS 0.13.0
		获取项目上次修改时间
	*/
	QDateTime YSSProject::getProjectLastModifyTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.LastModifyTime"), "yyyy-MM-dd hh:mm:ss");
	}

	/*!
		\since YSS 0.13.0
		获取项目设置的调试服务器ID。

		由于YSS项目目前假定只面向那些编码项目，而编码项目
		或多或少需要调试功能，因此提供了这个API。
	*/
	QString YSSProject::getProjectDebugServerID() {
		return d->ProjectConfig->getString("Project.DebugServerID");
	}

	/*!
		\since YSS 0.13.0
		设置项目的调试服务器ID为\a id

		由于YSS项目目前假定只面向那些编码项目，而编码项目
		或多或少需要调试功能，因此提供了这个API。
	*/
	void YSSProject::setProjectDebugServerID(const QString& id) {
		d->ProjectConfig->setString("Project.DebugServerID", id);
	}

	/*!
		\since YSS 0.13.0
		获取项目的版本号。从initProject初始化时，默认初始化为0.1.
	*/
	Visindigo::General::Version YSSProject::getProjectVersion() {
		return Visindigo::General::Version(d->ProjectConfig->getString("Project.Version"));
	}
	
	/*!
		\since YSS 0.13.0
		获取项目中为指定插件\a plugin保存的配置数据。

		请注意，获取配置数据并修改后，需要通过saveProjectConfigForPlugin重新设置到项目里。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getProjectConfigForPlugin(Visindigo::General::Plugin* plugin) {
		QString key = "MetaData." + plugin->getPluginID();
		return d->ProjectConfig->getObject(key);
	}

	/*!
		\since YSS 0.13.0
		获取项目中为指定插件\a plugin 保存的配置数据。这个重载是按插件ID而非指针索引的。

		请注意，获取配置数据并修改后，需要通过saveProjectConfigForPlugin重新设置到项目里。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getProjectConfigForPlugin(const QString& pluginID) {
		QString key = "MetaData." + pluginID;
		return d->ProjectConfig->getObject(key);
	}

	/*!
		\since YSS 0.13.0
		为指定插件\a plugin保存具有\a config 的数据。

		它被存储在YSSP文件的MetaData节点下。
	*/
	void YSSProject::saveProjectConfigForPlugin(Visindigo::General::Plugin* plugin, const Visindigo::Utility::JsonConfig& config) {
		QString key = "MetaData." + plugin->getPluginID();
		d->ProjectConfig->setObject(key, config);
	}

	/*!
		\since YSS 0.13.0
		为指定插件\a pluginID保存具有\a config 的数据。这个重载是按插件ID而非指针索引的。

		它被存储在YSSP文件的MetaData节点下。
	*/
	void YSSProject::saveProjectConfigForPlugin(const QString& pluginID, const Visindigo::Utility::JsonConfig& config) {
		QString key = "MetaData." + pluginID;
		d->ProjectConfig->setObject(key, config);
	}

	/*!
		\since YSS 0.13.0

		刷新项目的最后修改时间。它被直接刷新为调用此函数的时间。
	*/
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

	/*!
		\since YSS 0.13.0

		向项目中添加一个编辑器打开的文件路径\a filePath。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被打开了，但它并不会真的使编辑器打开这个文件。
	*/
	void YSSProject::addEditorOpenedFile(const QString& filePath) {
		QStringList files = getEditorOpenedFiles();
		if (files.contains(filePath)) {
			return;
		}
		QString relativePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), filePath);
		d->ProjectConfig->setString("Editor.OpenedFiles." + QString::number(d->ProjectConfig->keys("Editor.OpenedFiles").size()), relativePath);
	}

	/*!
		\since YSS 0.13.0
		设置项目中编辑器打开的文件列表为\a filePaths。
		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这些文件被打开了，但它并不会真的使编辑器打开这些文件。
	*/
	void YSSProject::setEditorOpenedFiles(const QStringList& filePaths) {
		removeAllEditorOpenedFiles();
		for (const QString& filePath : filePaths) {
			addEditorOpenedFile(filePath);
		}
	}

	/*!
		\since YSS 0.13.0
		从项目中移除一个编辑器打开的文件路径\a filePath。
		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被关闭了，但它并不会真的使编辑器关闭这个文件。
	*/
	void YSSProject::removeEditorOpenedFile(const QString& filePath) {
		QStringList files = getEditorOpenedFiles();
		QString absPath = QFileInfo(filePath).absoluteFilePath();
		if (!files.contains(absPath)) {
			return;
		}
		d->ProjectConfig->remove("Editor.OpenedFiles." + QString::number(files.indexOf(absPath)));
	}

	/*!
		\since YSS 0.13.0
		设置项目中编辑器当前聚焦的文件路径为\a filePath。
		这个API和YSSCore::Editor::FileServerManager::focusFile信号无关
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被聚焦了，但它并不会真的使编辑器聚焦这个文件。

		如果你希望YSS更改聚焦的文件，应该使用上面提到的这个信号。
	*/
	void YSSProject::setFocusedFile(const QString& abs_filePath) {
		if (abs_filePath.isEmpty()) {
			return;
		}
		if (!Visindigo::Utility::FileUtility::isFileExist(abs_filePath)) {
			yErrorF << "File not exist:" << abs_filePath;
			return;
		}
		QString relativePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), abs_filePath);
		d->ProjectConfig->setString("Editor.FocusedFile", relativePath);
	}

	/*!
		\since YSS 0.13.0
		获取项目中编辑器当前聚焦的文件路径。
	*/
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

	/*!
		\since YSS 0.13.0
		获取项目中编辑器当前聚焦的文件的文件名（不带路径）。
	*/
	QString YSSProject::getFocusedFileName() {
		QString abs_path = getFocusedFile();
		if (abs_path.isEmpty()) {
			return QString();
		}
		QFileInfo info(abs_path);
		return info.fileName();
	}

	/*!
		\since YSS 0.13.0
		从项目中移除所有编辑器打开的文件路径。
		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目没有文件被打开了，但它并不会真的使编辑器关闭所有文件。
	*/
	void YSSProject::removeAllEditorOpenedFiles() {
		d->ProjectConfig->remove("Editor.OpenedFiles");
	}

	/*!
		\since YSS 0.13.0
		获得当前项目的实例指针。
	*/
	YSSProject* YSSProject::getCurrentProject() {
		return YSSProjectPrivate::CurrentProject;
	}

	/*!
		\since YSS 0.13.0
		设置当前项目实例为\a project。
	*/
	void YSSProject::setCurrentProject(YSSProject* project) {
		if (YSSProjectPrivate::CurrentProject != nullptr) {
			delete YSSProjectPrivate::CurrentProject;
		}
		YSSProjectPrivate::CurrentProject = project;
	}
}