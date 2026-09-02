#include <algorithm>
#include <functional>
#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qstring.h>
#include <General/Log.h>
#include <General/Plugin.h>
#include <General/Version.h>
#include <General/VirtualStorage.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include "Editor/DebugServerManager.h"
#include "Editor/VirtualFilePath.h"
#include "General/YSSLogger.h"
#include "General/YSSProject.h"
namespace YSSCore::General {
	class YSSProjectPrivate {
		friend class YSSProject;
	protected:
		static YSSProject* CurrentProject;
		Visindigo::Utility::JsonConfig* ProjectConfig = nullptr;
		QString ConfigPath;
		Visindigo::Utility::VirtualStorage* BackupStorage = nullptr;
		qint32 BackupMaxCount = 50;

		YSSProjectPrivate() {
			ProjectConfig = new Visindigo::Utility::JsonConfig();
		}
		~YSSProjectPrivate() {
			if (ProjectConfig != nullptr) {
				delete ProjectConfig;
			}
			if (BackupStorage != nullptr) {
				delete BackupStorage;
			}
		}

		void updateLastModifyTime() {
			ProjectConfig->setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		}

		QString yssDir() const {
			QFileInfo info(ConfigPath);
			return info.absolutePath() + "/.yss";
		}

		void ensureYssDir() const {
			QString path = yssDir();
			if (!Visindigo::Utility::FileUtility::isDirExist(path)) {
				Visindigo::Utility::FileUtility::createDir(path);
			}
		}

		Visindigo::Utility::VirtualStorage* getBackupStorage() {
			if (BackupStorage == nullptr) {
				ensureYssDir();
				BackupStorage = new Visindigo::Utility::VirtualStorage(
					yssDir() + "/backup.db");
			}
			return BackupStorage;
		}

		/*!
			关闭并销毁 \a storage，删除对应数据库文件，然后重建新的空库。
		*/
		void resetStorage(Visindigo::Utility::VirtualStorage*& storage,
			const QString& dbFileName) {
			if (storage != nullptr) {
				delete storage;
				storage = nullptr;
			}
			QString dbPath = yssDir() + "/" + dbFileName;
			QFile::remove(dbPath);
			storage = new Visindigo::Utility::VirtualStorage(dbPath);
		}

		/*!
			将路径中的 "." 替换为 "|"（Windows 路径非法字符），
			避免 JsonConfig 的 "." 键分隔符误切路径段。
			由于 "|" 不会出现在任何合法项目路径中，转换完全可逆。
		*/
		static QString escapeDots(const QString& path) {
			QString result = path;
			result.replace(QLatin1Char('.'), QLatin1Char('|'));
			return result;
		}

		/*!
			构造备份虚拟路径：
			escapedPath/yyyyMMdd_hh_mm_ss.ext
		*/
		static QString backupVirtualPath(const QString& inProjRelativePth,
			const QDateTime& time,
			const QString& extension) {
			QString folder = escapeDots(inProjRelativePth);
			QString ts = time.toString("yyyyMMdd_hh_mm_ss");
			return folder + "/" + ts + "." + extension;
		}

		/*!
			构造回收站虚拟路径：同备份路径规则。
		*/
		static QString trashVirtualPath(const QString& inProjRelativePth,
			const QDateTime& time,
			const QString& extension) {
			return backupVirtualPath(inProjRelativePth, time, extension);
		}

		/*!
			从备份虚拟路径的文件名中解析出 QDateTime。
			文件名格式：yyyyMMdd_hh_mm_ss.ext
		*/
		static QDateTime parseTimestampFromName(const QString& fileName) {
			// 取第一个 '.' 之前的部分尝试解析
			int dotPos = fileName.indexOf(QLatin1Char('.'));
			QString tsPart = (dotPos >= 0) ? fileName.left(dotPos) : fileName;
			return QDateTime::fromString(tsPart, "yyyyMMdd_hh_mm_ss");
		}

		/*!
			获取给定路径的原始扩展名（不含点）。
		*/
		static QString fileExtension(const QString& path) {
			int dotPos = path.lastIndexOf(QLatin1Char('.'));
			if (dotPos >= 0) {
				return path.mid(dotPos + 1);
			}
			return QString();
		}
	};

	YSSProject* YSSProjectPrivate::CurrentProject = nullptr;

	/*!
		\class YSSCore::General::YSSProject
		\brief 这个类代表YSS项目数据.
		\since YSS 0.13.0
		\inmodule YSSCore

		YSSProject类代表YSS项目数据。值得注意的是，这个类目前是个非QObject类，
		它是作为project.yssp文件的API操作以及读写使用，只负责数据操作，
		不负责在程序中充当一个关于项目各项内容更改时的数据集散中心。

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
		从 \a configPath 加载YSS项目
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
		return 当前项目的配置文件路径。
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
		在 \a folder 指定的文件夹初始化项目，名称为 \a name

		如果 \a folder 不存在，那么会自动创建沿途所有不存在的文件夹。

		如果 \a folder 不为空，会创建失败。

		只要可以创建，这函数执行之后，configPath就被自动设置为指定的位置。

		如果要通过YSSProject初始化YSS项目，一般推荐首先调用此函数以建立项目文件夹，
		稍后再进行其他初始化操作。

		return 是否成功初始化项目
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
		d->ProjectConfig->setString("Project.Version", "0.2");
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
		return 当前项目的文件夹路径。

		与getProjectPath有所不同的是，这个函数固定按
		绝对路径返回。
	*/
	QString YSSProject::getProjectFolder() {
		QFileInfo info(d->ConfigPath);
		return info.absolutePath();
	}

	/*!
		\since YSS 0.13.0
		return 当前项目的配置文件路径。

		如果项目加载时是有效的，那这个返回值就等同于设置值。
		即，它是绝对路径还是相对路径取决于设置时的原样。
	*/
	QString YSSProject::getProjectPath() {
		return d->ConfigPath;
	}

	/*!
		\since YSS 0.13.0
		return 当前项目的图标路径。

		这个图标路径忠实返回项目文件设置的值，不管他
		是否真的有效，以及是什么格式的路径。
	*/
	QString YSSProject::getProjectIconPath() {
		return d->ProjectConfig->getString("Project.IconPath");
	}

	/*!
		\since YSS 0.13.0
		return 项目作者设置。
	*/
	QString YSSProject::getProjectAuthor() {
		return d->ProjectConfig->getString("Project.Author");
	}

	/*!
		\since YSS 0.13.0
		设置项目名称为 \a name
	*/
	void YSSProject::setProjectName(const QString& name) {
		d->ProjectConfig->setString("Project.Name", name);
	}

	/*!
		\since YSS 0.13.0
		设置项目描述为 \a description
	*/
	void YSSProject::setProjectDescription(const QString& description) {
		d->ProjectConfig->setString("Project.Description", description);
	}

	/*!
		\since YSS 0.13.0
		设置项目图标路径为 \a iconPath
	*/
	void YSSProject::setProjectIconPath(const QString& iconPath) {
		d->ProjectConfig->setString("Project.IconPath", iconPath);
	}

	/*!
		\since YSS 0.13.0
		设置项目作者为 \a author
	*/
	void YSSProject::setProjectAuthor(const QString& author) {
		d->ProjectConfig->setString("Project.Author", author);
	}

	/*!
		\since YSS 0.13.0
		return 项目创建时间
	*/
	QDateTime YSSProject::getProjectCreateTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.CreateTime"), "yyyy-MM-dd hh:mm:ss");
	}

	/*!
		\since YSS 0.13.0
		return 项目上次修改时间
	*/
	QDateTime YSSProject::getProjectLastModifyTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.LastModifyTime"), "yyyy-MM-dd hh:mm:ss");
	}

	/*!
		\since YSS 0.13.0
		return 项目设置的调试服务器ID。

		由于YSS项目目前假定只面向那些编码项目，而编码项目
		或多或少需要调试功能，因此提供了这个API。
	*/
	QString YSSProject::getProjectDebugServerID() {
		return d->ProjectConfig->getString("Project.DebugServerID");
	}

	/*!
		\since YSS 0.13.0
		设置项目的调试服务器ID为 \a id

		由于YSS项目目前假定只面向那些编码项目，而编码项目
		或多或少需要调试功能，因此提供了这个API。
	*/
	void YSSProject::setProjectDebugServerID(const QString& id) {
		d->ProjectConfig->setString("Project.DebugServerID", id);
		emit YSSCore::Editor::DebugServerManager::getInstance()->projectDebugServerChanged(id);
	}

	/*!
		\since YSS 0.13.0
		return 项目的版本号。从initProject初始化时，默认初始化为0.1.
	*/
	Visindigo::General::Version YSSProject::getProjectVersion() {
		return Visindigo::General::Version(d->ProjectConfig->getString("Project.Version"));
	}

	/*!
		\since YSS 0.13.0
		return 项目中为指定插件 \a plugin 保存的配置数据。

		请注意，获取配置数据并修改后，需要通过saveProjectConfigForPlugin重新设置到项目里。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getProjectConfigForPlugin(Visindigo::General::Plugin* plugin) {
		QString key = "MetaData." + plugin->getPluginID();
		return d->ProjectConfig->getObject(key);
	}

	/*!
		\since YSS 0.13.0
		return 项目中为指定插件 \a pluginID 保存的配置数据。这个重载是按插件ID而非指针索引的。

		请注意，获取配置数据并修改后，需要通过saveProjectConfigForPlugin重新设置到项目里。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getProjectConfigForPlugin(const QString& pluginID) {
		QString key = "MetaData." + pluginID;
		return d->ProjectConfig->getObject(key);
	}

	/*!
		\since YSS 0.17.0

		return 项目中为树形布局保存的配置数据。这个配置数据是一个JsonConfig对象，包含了树形布局的相关设置。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getTreeLayoutConfig() {
		return d->ProjectConfig->getObject("TreeLayout");
	}

	/*!
		\since YSS 0.17.0
		为树形布局设置配置数据。这个配置数据是一个JsonConfig对象，包含了树形布局的相关设置。
	*/
	void YSSProject::setTreeLayoutConfig(const Visindigo::Utility::JsonConfig& config) {
		d->ProjectConfig->setObject("TreeLayout", config);
	}

	/*!
		\since YSS 0.17.0

		return 项目中是否有树形布局的配置数据。
	*/
	bool YSSProject::hasTreeLayoutConfig() {
		return d->ProjectConfig->contains("TreeLayout");
	}

	/*!
		\since YSS 0.13.0
		为指定插件 \a plugin 保存具有 \a config 的数据。

		它被存储在YSSP文件的MetaData节点下。
	*/
	void YSSProject::saveProjectConfigForPlugin(Visindigo::General::Plugin* plugin, const Visindigo::Utility::JsonConfig& config) {
		QString key = "MetaData." + plugin->getPluginID();
		d->ProjectConfig->setObject(key, config);
	}

	/*!
		\since YSS 0.13.0
		为指定插件 \a pluginID 保存具有 \a config 的数据。这个重载是按插件ID而非指针索引的。

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

	/*!
		\since YSS 0.13.0

		return 项目中编辑器当前打开的文件路径列表。对于真实文件路径，永远返回绝对路径；对于虚拟文件路径，返回原始的虚拟文件路径。

		这个函数不区分区域，它会自动检索所有区域的打开文件列表，返回所有文件路径。

		如果需要区分区域的版本，请使用getEditorOpenedFilesInArea函数。
	*/
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
		QStringList extraKeys = d->ProjectConfig->keys("Editor.OpenedFilesExtra");
		for (const QString& areaID : extraKeys) {
			QStringList areaFileKeys = d->ProjectConfig->keys("Editor.OpenedFilesExtra." + areaID);
			for (const QString& key : areaFileKeys) {
				QString filePath = d->ProjectConfig->getString("Editor.OpenedFilesExtra." + areaID + "." + key);
				if (filePath.startsWith("./")) {
					filePath = getProjectFolder() + "/" + filePath.mid(2);
				}
				files.append(filePath);
			}
		}
		return files;
	}

	/*!
		\since YSS 0.17.0

		return 项目中编辑器当前打开的文件路径列表，按区域区分。对于真实文件路径，永远返回绝对路径；对于虚拟文件路径，返回原始的虚拟文件路径。
	*/
	QMap<QString, QStringList> YSSProject::getEditorOpenedFilesInArea() {
		QMap<QString, QStringList> areaFiles;
		QStringList keys = d->ProjectConfig->keys("Editor.OpenedFiles");
		areaFiles["0"] = QStringList();
		for (const QString& key : keys) {
			QString filePath = d->ProjectConfig->getString("Editor.OpenedFiles." + key);
			if (filePath.startsWith("./")) {
				filePath = getProjectFolder() + "/" + filePath.mid(2);
			}
			areaFiles["0"].append(filePath);
		}
		QStringList extraKeys = d->ProjectConfig->keys("Editor.OpenedFilesExtra");
		for (const QString& areaID : extraKeys) {
			QString realID = areaID;
			realID.remove(0, 2); // 去掉前缀 "s_"
			areaFiles[realID] = QStringList();
			QStringList areaFileKeys = d->ProjectConfig->keys("Editor.OpenedFilesExtra." + areaID);
			for (const QString& key : areaFileKeys) {
				QString filePath = d->ProjectConfig->getString("Editor.OpenedFilesExtra." + areaID + "." + key);
				if (filePath.startsWith("./")) {
					filePath = getProjectFolder() + "/" + filePath.mid(2);
				}
				areaFiles[realID].append(filePath);
			}
		}
		return areaFiles;
	}

	/*!
		\since YSS 0.17.0
		\a filePath 编辑器打开的文件路径，绝对路径或相对于项目文件夹的相对路径均可。
		\a areaID 编辑器打开文件所在的区域ID，默认为空字符串。

		向项目中添加一个编辑器打开的文件路径。如果这个路径最终落在项目文件夹内，则
		转换为相对路径存储，否则以绝对路径存储。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被打开了，但它并不会真的使编辑器打开这个文件。

		\note 在0.17.0支持多窗口之前，没有areaID参数。自0.17.0起，编辑器可能有多个窗口，每个窗口都有自己的打开文件列表。
		为了兼容考虑，如果areaID为空或"0"，则表示默认的主窗口，且继续保存在Editor.OpenedFiles节点下，如果areaID不为空且不为"0"，
		则表示其他窗口，保存到Editor.OpenedFilesExtra.<areaID>节点下。
	*/
	void YSSProject::addEditorOpenedFile(const QString& filePath, const QString& areaID) {
		QStringList files = getEditorOpenedFiles();
		QString absInput = filePath;
		if (absInput.startsWith("./")) {
			absInput = getProjectFolder() + "/" + absInput.mid(2);
		}
		if (files.contains(absInput)) {
			return;
		}
		if (not YSSCore::Editor::VirtualFilePath::isVirtualFilePath(filePath)) {
			QString relativePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), filePath);
			if (areaID.isEmpty() || areaID == "0") {
				d->ProjectConfig->setString("Editor.OpenedFiles." + QString::number(d->ProjectConfig->keys("Editor.OpenedFiles").size()), relativePath);
			} else {
				d->ProjectConfig->setString("Editor.OpenedFilesExtra.s_" + areaID + "." + QString::number(d->ProjectConfig->keys("Editor.OpenedFilesExtra.s_" + areaID).size()), relativePath);
			}
		}
		else {
			if (areaID.isEmpty() || areaID == "0") {
				d->ProjectConfig->setString("Editor.OpenedFiles." + QString::number(d->ProjectConfig->keys("Editor.OpenedFiles").size()), filePath);
			}
			else {
				d->ProjectConfig->setString("Editor.OpenedFilesExtra.s_" + areaID + "." + QString::number(d->ProjectConfig->keys("Editor.OpenedFilesExtra.s_" + areaID).size()), filePath);
			}
		}
	}

	/*!
		\since YSS 0.17.0
		\a filePath 编辑器打开的文件路径，绝对路径或相对于项目文件夹的相对路径均可。
		\a fromID 编辑器打开文件所在的原区域ID，默认为空字符串。
		\a toID 编辑器打开文件所在的新区域ID，默认为空字符串。

		将项目中一个编辑器打开的文件路径从一个区域移动到另一个区域。如果fromID中没有此文件路径，则不会移动。
		如果fromID和toID相同，不做任何操作。
	*/
	void YSSProject::moveEditorOpenedFile(const QString& filePath, const QString& fromID, const QString& toID) {
		if (fromID == toID) {
			return;
		}
		QStringList files = getEditorOpenedFiles();
		QString absInput = filePath;
		if (absInput.startsWith("./")) {
			absInput = getProjectFolder() + "/" + absInput.mid(2);
		}
		if (not files.contains(absInput)) {
			return;
		}
		removeEditorOpenedFile(filePath);
		addEditorOpenedFile(filePath, toID);
	}
	/*!
		\since YSS 0.13.0
		\a filePaths 编辑器打开的文件路径列表，绝对路径或相对于项目文件夹的相对路径均可。

		有关其中每个文件路径的说明，请参见addEditorOpenedFile函数。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这些文件被打开了，但它并不会真的使编辑器打开这些文件。

		这个函数将把所有文件都添加到默认区域。这个函数没有区分区域的能力。若要区分区域，请用setEditorOpenedFilesInArea函数。
	*/
	void YSSProject::setEditorOpenedFiles(const QStringList& filePaths) {
		removeAllEditorOpenedFiles();
		for (const QString& filePath : filePaths) {
			addEditorOpenedFile(filePath);
		}
	}

	/*!
		\since YSS 0.17.0
		\a filePathsInArea 编辑器打开的文件路径列表，按区域区分。绝对路径或相对于项目文件夹的相对路径均可。

		有关其中每个文件路径的说明，请参见addEditorOpenedFile函数。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这些文件被打开了，但它并不会真的使编辑器打开这些文件。
	*/
	void YSSProject::setEditorOpenedFilesInArea(const QMap<QString, QStringList>& filePathsInArea) {
		removeAllEditorOpenedFiles();
		for (auto key : filePathsInArea.keys()) {
			for (const QString& filePath : filePathsInArea[key]) {
				addEditorOpenedFile(filePath, key);
			}
		}
	}

	/*!
		\since YSS 0.13.0
		\a filePath 编辑器打开的文件路径，绝对路径或相对于项目文件夹的相对路径均可。

		移除项目中一个编辑器打开的文件路径。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被关闭了，但它并不会真的使编辑器关闭这个文件。

		这个函数不区分区域，它会自动检索所有区域的打开文件列表，找到匹配的文件路径后移除。
	*/
	void YSSProject::removeEditorOpenedFile(const QString& filePath) {
		QStringList files = getEditorOpenedFiles();
		QString absInput = filePath;
		if (absInput.startsWith("./")) {
			absInput = getProjectFolder() + "/" + absInput.mid(2);
		}
		if (not files.contains(absInput)) {
			return;
		}
		QString areaID = getFileAreaID(filePath);
		if (areaID.isEmpty() || areaID == "0") {
			QStringList keys = d->ProjectConfig->keys("Editor.OpenedFiles");
			for (const QString& key : keys) {
				QString filePathInConfig = d->ProjectConfig->getString("Editor.OpenedFiles." + key);
				if (filePathInConfig.startsWith("./")) {
					filePathInConfig = getProjectFolder() + "/" + filePathInConfig.mid(2);
				}
				if (filePathInConfig == absInput) {
					d->ProjectConfig->remove("Editor.OpenedFiles." + key);
					break;
				}
			}
		}
		else {
			QStringList keys = d->ProjectConfig->keys("Editor.OpenedFilesExtra.s_" + areaID);
			for (const QString& key : keys) {
				QString filePathInConfig = d->ProjectConfig->getString("Editor.OpenedFilesExtra.s_" + areaID + "." + key);
				if (filePathInConfig.startsWith("./")) {
					filePathInConfig = getProjectFolder() + "/" + filePathInConfig.mid(2);
				}
				if (filePathInConfig == absInput) {
					d->ProjectConfig->remove("Editor.OpenedFilesExtra.s_" + areaID + "." + key);
					break;
				}
			}
		}
	}

	/*!
		\since YSS 0.13.0
		\a abs_filePath 编辑器聚焦的文件路径，绝对路径或相对于项目文件夹的相对路径均可。

		设置项目中编辑器当前聚焦的文件路径。

		这个API和YSSCore::Editor::FileServerManager::focusFile信号无关
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被聚焦了，但它并不会真的使编辑器聚焦这个文件。

		如果你希望YSS更改聚焦的文件，应该使用上面提到的这个信号。
	*/
	void YSSProject::setFocusedFile(const QString& abs_filePath) {
		if (abs_filePath.isEmpty()) {
			return;
		}
		auto vfs = YSSCore::Editor::VirtualFilePath(abs_filePath);
		if (vfs.isValid()) {
			yInfo << "Focused file is a virtual file path, store as absolute path:" << abs_filePath;
			d->ProjectConfig->setString("Editor.FocusedFile", abs_filePath);
		}
		else {
			if (not Visindigo::Utility::FileUtility::isFileExist(abs_filePath)) {
				yErrorF << "File not exist:" << abs_filePath;
				return;
			}
			QString relativePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(getProjectFolder(), abs_filePath);
			d->ProjectConfig->setString("Editor.FocusedFile", relativePath);
		}
	}

	/*!
		\since YSS 0.17.0

		return 指定文件所在的区域ID，如果文件不在任何区域中打开，则返回空字符串。
	*/
	QString YSSProject::getFileAreaID(const QString& abs_filePath) {
		QString absInput = abs_filePath;
		if (absInput.startsWith("./")) {
			absInput = getProjectFolder() + "/" + absInput.mid(2);
		}
		if (d->ProjectConfig->contains("Editor.OpenedFiles")) {
			QStringList keys = d->ProjectConfig->keys("Editor.OpenedFiles");
			for (const QString& key : keys) {
				QString filePath = d->ProjectConfig->getString("Editor.OpenedFiles." + key);
				if (filePath.startsWith("./")) {
					filePath = getProjectFolder() + "/" + filePath.mid(2);
				}
				if (filePath == absInput) {
					return "0"; // 默认区域
				}
			}
		}
		if (d->ProjectConfig->contains("Editor.OpenedFilesExtra")) {
			QStringList areaKeys = d->ProjectConfig->keys("Editor.OpenedFilesExtra");
			for (const QString& areaID : areaKeys) {
				QStringList fileKeys = d->ProjectConfig->keys("Editor.OpenedFilesExtra." + areaID);
				for (const QString& key : fileKeys) {
					QString filePath = d->ProjectConfig->getString("Editor.OpenedFilesExtra." + areaID + "." + key);
					if (filePath.startsWith("./")) {
						filePath = getProjectFolder() + "/" + filePath.mid(2);
					}
					if (filePath == absInput) {
						QString realAreaID = areaID;
						realAreaID.remove(0, 2); // 去掉前缀 "s_"
						return realAreaID;
					}
				}
			}
		}
		return QString(); // 不在任何区域中
	}

	/*!
		\since YSS 0.13.0

		return 项目中编辑器当前聚焦的文件路径。对于真实文件路径，永远返回绝对路径；对于虚拟文件路径，返回原始的虚拟文件路径。
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

		return 项目中编辑器当前聚焦的文件的文件名（不带路径）。对于虚拟文件路径，返回虚拟文件路径的名称部分。对于真实文件路径，返回真实文件的文件名。
	*/
	QString YSSProject::getFocusedFileName() {
		QString abs_path = getFocusedFile();
		if (abs_path.isEmpty()) {
			return QString();
		}
		if (YSSCore::Editor::VirtualFilePath::isVirtualFilePath(abs_path)) {
			return YSSCore::Editor::VirtualFilePath(abs_path).getFileName();
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
		d->ProjectConfig->remove("Editor.OpenedFilesExtra");
	}

	/*!
		\since YSS 0.17.0
		\a areaID 编辑器打开文件所在的区域ID，默认为空字符串。

		从项目中移除指定区域的所有编辑器打开的文件路径。
	*/
	void YSSProject::removeEditorOpenedFilesInArea(const QString& areaID) {
		if (areaID.isEmpty() || areaID == "0") {
			d->ProjectConfig->remove("Editor.OpenedFiles");
		}
		else {
			d->ProjectConfig->remove("Editor.OpenedFilesExtra.s_" + areaID);
		}
	}

	/*!
		\since YSS 0.13.0
		return 当前项目的实例指针。
	*/
	YSSProject* YSSProject::getCurrentProject() {
		return YSSProjectPrivate::CurrentProject;
	}

	/*!
		\since YSS 0.13.0
		设置当前项目实例为 \a project 。
	*/
	void YSSProject::setCurrentProject(YSSProject* project) {
		if (YSSProjectPrivate::CurrentProject != nullptr) {
			delete YSSProjectPrivate::CurrentProject;
		}
		YSSProjectPrivate::CurrentProject = project;
	}

	// ============================================================
	//  备份功能
	// ============================================================

	/*!
		为项目内相对路径 \a inProjRelativePth 指向的文件创建一份备份。
		文件内容从磁盘按原路径读取，存入备份虚拟存储中。
	*/
	void YSSProject::createFileBackup(const QString& inProjRelativePth) {
		QString absPath = getProjectFolder() + "/" + inProjRelativePth;
		if (!Visindigo::Utility::FileUtility::isFileExist(absPath)) {
			yErrorF << "File not found for backup:" << absPath;
			return;
		}

		QByteArray content;
		{
			QFile file(absPath);
			if (file.open(QIODevice::ReadOnly)) {
				content = file.readAll();
			}
		}
		QDateTime now = QDateTime::currentDateTime();
		QString ext = d->fileExtension(inProjRelativePth);
		QString vpth = d->backupVirtualPath(inProjRelativePth, now, ext);

		d->getBackupStorage()->saveFile(vpth, content);

		// 清理超出上限的旧备份
		QString folder = d->escapeDots(inProjRelativePth);
		Visindigo::Utility::JsonConfig listing = d->getBackupStorage()->listFolder(folder, false);
		Visindigo::Utility::JsonConfig filesObj = listing.getObject("files");
		QStringList fileNames = filesObj.keys();
		if (fileNames.size() > d->BackupMaxCount) {
			// 按时间戳排序，保留最新的 BackupMaxCount 个
			QList<QPair<QDateTime, QString>> sorted;
			for (const QString& name : fileNames) {
				QDateTime ts = d->parseTimestampFromName(name);
				if (ts.isValid()) {
					sorted.append({ts, name});
				}
			}
			std::sort(sorted.begin(), sorted.end(),
				[](const QPair<QDateTime, QString>& a,
				   const QPair<QDateTime, QString>& b) {
					return a.first > b.first; // 降序，最新的在前
				});
			for (int i = d->BackupMaxCount; i < sorted.size(); ++i) {
				d->getBackupStorage()->removeFile(folder + "/" + sorted[i].second);
			}
		}
	}

	/*!
		返回 \a inProjRelativePth 的所有备份时间列表。
	*/
	QList<QDateTime> YSSProject::getFileBackupList(const QString& inProjRelativePth) {
		QList<QDateTime> result;
		QString folder = d->escapeDots(inProjRelativePth);
		Visindigo::Utility::JsonConfig listing = d->getBackupStorage()->listFolder(folder, false);
		Visindigo::Utility::JsonConfig filesObj = listing.getObject("files");
		QStringList fileNames = filesObj.keys();

		for (const QString& name : fileNames) {
			QDateTime ts = d->parseTimestampFromName(name);
			if (ts.isValid()) {
				result.append(ts);
			}
		}
		std::sort(result.begin(), result.end(),
		          std::greater<QDateTime>());
		return result;
	}

	/*!
		获取 \a inProjRelativePth 在 \a backupTime 时的备份内容。
	*/
	QByteArray YSSProject::getFileBackupContent(const QString& inProjRelativePth,
	                                             const QDateTime& backupTime) {
		QString ext = d->fileExtension(inProjRelativePth);
		QString vpth = d->backupVirtualPath(inProjRelativePth, backupTime, ext);
		return d->getBackupStorage()->readFile(vpth);
	}

	/*!
		将 \a inProjRelativePth 在 \a backupTime 的备份还原到磁盘。
		若 \a restoreToPath 非空，则还原到指定路径；
		若 \a overwrite 为 false 且目标已存在，则不覆盖返回 false。
	*/
	bool YSSProject::restoreFileBackup(const QString& inProjRelativePth,
	                                    const QDateTime& backupTime,
	                                    const QString& restoreToPath, bool overwrite) {
		QString ext = d->fileExtension(inProjRelativePth);
		QString vpth = d->backupVirtualPath(inProjRelativePth, backupTime, ext);
		QByteArray content = d->getBackupStorage()->readFile(vpth);
		if (content.isEmpty()) {
			return false;
		}

		QString targetPath = restoreToPath.isEmpty()
			? (getProjectFolder() + "/" + inProjRelativePth)
			: restoreToPath;

		if (!overwrite && Visindigo::Utility::FileUtility::isFileExist(targetPath)) {
			return false;
		}

		{
			QFile file(targetPath);
			if (file.open(QIODevice::WriteOnly)) {
				file.write(content);
			}
		}
		return true;
	}

	/*!
		删除 \a inProjRelativePth 在 \a backupTime 的单份备份。
	*/
	void YSSProject::removeFileBackup(const QString& inProjRelativePth,
	                                   const QDateTime& backupTime) {
		QString ext = d->fileExtension(inProjRelativePth);
		QString vpth = d->backupVirtualPath(inProjRelativePth, backupTime, ext);
		d->getBackupStorage()->removeFile(vpth);
	}

	/*!
		删除 \a inProjRelativePth 的所有备份。
	*/
	void YSSProject::removeFileBackup(const QString& inProjRelativePth) {
		QString folder = d->escapeDots(inProjRelativePth);
		Visindigo::Utility::JsonConfig listing = d->getBackupStorage()->listFolder(folder, false);
		Visindigo::Utility::JsonConfig filesObj = listing.getObject("files");
		QStringList fileNames = filesObj.keys();
		for (const QString& name : fileNames) {
			d->getBackupStorage()->removeFile(folder + "/" + name);
		}
	}

	/*!
		删除项目中所有文件的全部备份。
		直接销毁数据库文件并重建空库，避免逐条 SQL DELETE 的开销。
	*/
	void YSSProject::removeAllFileBackup() {
		d->resetStorage(d->BackupStorage, "backup.db");
	}

	/*!
		设置每个文件保留的最大备份数量。超过上限时，最旧的备份会被自动清理。
	*/
	void YSSProject::setFileBackupMaxCount(qint32 count) {
		d->BackupMaxCount = (count > 0) ? count : 1;
	}

	/*!
		return 当前设置的每文件最大备份数量。
	*/
	qint32 YSSProject::getFileBackupMaxCount() {
		return d->BackupMaxCount;
	}

	/*!
		return 所有已备份文件的原始项目内相对路径列表。
	*/
	QStringList YSSProject::getAllFileBackups() {
		QStringList result;
		Visindigo::Utility::JsonConfig root =
			d->getBackupStorage()->listFolder("", false);
		Visindigo::Utility::JsonConfig foldersObj = root.getObject("sub_folders");
		QStringList escapedNames = foldersObj.keys();
		for (const QString& escaped : escapedNames) {
			QString origPath = escaped;
			origPath.replace(QLatin1Char('|'), QLatin1Char('.'));
			result.append(origPath);
		}
		return result;
	}
}