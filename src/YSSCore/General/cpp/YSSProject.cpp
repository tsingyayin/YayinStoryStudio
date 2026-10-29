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
#include <Utility/FileOperation.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include "Editor/DebugServerManager.h"
#include "Editor/VirtualFilePath.h"
#include "General/YSSLogger.h"
#include "General/YSSProject.h"
#include "General/YSSUniqueFileSystem.h"
namespace YSSCore::General {
	class YSSProjectPrivate {
		friend class YSSProject;
	protected:
		static const Visindigo::General::Version ProjectVersion;
		static const QMap<Visindigo::General::Version, std::tuple<Visindigo::General::Version, Visindigo::General::Version>> ProjectVersionRange;
		static YSSProject* CurrentProject;
		// 远端项目里记录本机绝对路径时使用的占位前缀，见 uriToStr 的说明。
		static const QString RemoteLocalPrefix;
		Visindigo::Utility::JsonConfig ProjectConfig;
		YSSURI ConfigPath;
		bool Loaded = false;
		bool Saving = false;
		// 写入进行中再次请求保存时不排队多条请求，只记下"结束后还要再写一次"。
		bool SavePending = false;
		YSSProject::LoadError LastLoadError = YSSProject::LoadError::None;
		Visindigo::Utility::VirtualStorage* BackupStorage = nullptr;
		qint32 BackupMaxCount = 50;

		~YSSProjectPrivate() {
			if (BackupStorage != nullptr) {
				delete BackupStorage;
			}
		}

		void updateLastModifyTime() {
			ProjectConfig.setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		}

		/*
			项目文件夹，即配置文件所在目录。ConfigPath 无效时返回无效 URI。
		*/
		YSSURI projectFolder() const {
			return ConfigPath.parent();
		}

		/*
			项目缓存目录 \c{.yss}。它跟随项目所在的后端：本地项目就是本地目录，
			远端项目就是远端主机上的同名目录。
		*/
		YSSURI yssDir() const {
			const YSSURI folder = projectFolder();
			if (not folder.isValid()) {
				return YSSURI();
			}
			return folder.resolve(".yss");
		}

		/*
			缓存目录的本地路径。备份库是本地 SQLite 文件，只能落在本机，
			因此远端项目暂时拿不到它。
		*/
		QString yssDirLocalPath() const {
			const YSSURI dir = yssDir();
			if (not dir.isLocal()) {
				return QString();
			}
			return dir.getLocalPath();
		}

		void ensureYssDir() const {
			const QString path = yssDirLocalPath();
			if (path.isEmpty()) {
				return;
			}
			if (!Visindigo::Utility::FileUtility::isDirExist(path)) {
				Visindigo::Utility::FileUtility::createDir(path);
			}
		}

		Visindigo::Utility::VirtualStorage* getBackupStorage() {
			if (BackupStorage == nullptr) {
				ensureYssDir();
				const QString dir = yssDirLocalPath();
				if (dir.isEmpty()) {
					yErrorF << "Project backup storage needs a local project folder, current path: "
						<< ConfigPath.toString();
					return nullptr;
				}
				BackupStorage = new Visindigo::Utility::VirtualStorage(dir + "/backup.db");
			}
			return BackupStorage;
		}

		/*
			关闭并销毁 \a storage，删除对应数据库文件，然后重建新的空库。
		*/
		void resetStorage(Visindigo::Utility::VirtualStorage*& storage,
			const QString& dbFileName) {
			if (storage != nullptr) {
				delete storage;
				storage = nullptr;
			}
			const QString dir = yssDirLocalPath();
			if (dir.isEmpty()) {
				yErrorF << "Project backup storage needs a local project folder, current path: "
					<< ConfigPath.toString();
				return;
			}
			QString dbPath = dir + "/" + dbFileName;
			QFile::remove(dbPath);
			storage = new Visindigo::Utility::VirtualStorage(dbPath);
		}

		/*
			两个 URI 是否指向同一个 SSH 连接身份（user@host:port）。
		*/
		static bool sameAuthority(const YSSURI& left, const YSSURI& right) {
			const std::optional<Visindigo::Network::SshPath> leftPath = left.getSshPath();
			const std::optional<Visindigo::Network::SshPath> rightPath = right.getSshPath();
			if (not leftPath or not rightPath) {
				return false;
			}
			return leftPath->getAuthority() == rightPath->getAuthority();
		}

		/*
			\a uri 相对 \a folder 的存储形式。
		*/
		static QString relativeTo(const YSSURI& uri, const YSSURI& folder) {
			const QString relative = uri.relativePathTo(folder);
			if (relative.isEmpty()) {
				return QStringLiteral("./");
			}
			return QStringLiteral("./") + relative;
		}

		/*
			把 \a uri 转换成项目配置文件中的存储形式。

			ConfigPath 为本地路径时：
			\list
			\li 落在项目文件夹内的路径记作 \c{./相对路径}；
			\li 其余本地路径记作本地绝对路径；
			\li 远端路径记作 \c{ssh://} 远程绝对路径。
			\endlist

			ConfigPath 为远端路径时：
			\list
			\li 与之同一 authority 的远端路径按上面的本地规则处理，即项目文件夹内记
				\c{./相对路径}，否则记远端绝对路径；
			\li 其它 authority 的远端路径记作远程绝对路径；
			\li 本地路径加上 \l{RemoteLocalPrefix} 前缀后记录，表示"执行保存的这台
				机器上的绝对路径"。
			\endlist

			虚拟文档与 QRC 资源没有项目文件夹语义，始终原样记录。
		*/
		QString uriToStr(const YSSURI& uri) const {
			if (not uri.isValid()) {
				return QString();
			}
			if (uri.isVirtual() or uri.isResource()) {
				return uri.toString();
			}
			const YSSURI folder = projectFolder();
			const bool inProject = folder.isValid() and uri.isUnder(folder);
			if (not ConfigPath.isRemote()) {
				if (uri.isRemote()) {
					return uri.toString();
				}
				return inProject ? relativeTo(uri, folder) : uri.getPath();
			}
			// ConfigPath 为远端
			if (uri.isRemote()) {
				if (sameAuthority(uri, ConfigPath) and inProject) {
					return relativeTo(uri, folder);
				}
				return uri.toString();
			}
			return RemoteLocalPrefix + uri.getPath();
		}

		/*
			\l{uriToStr()} 的逆过程。

			\c{./} 开头的文本按项目文件夹解析，因此 ConfigPath 为远端时得到同一个
			authority 的远端 URI；其余文本交给 \l{YSSURI::fromString()}。带
			\l{RemoteLocalPrefix} 前缀的文本会被还原成本机绝对路径——无论当前
			ConfigPath 是本地还是远端：配置文件里写着"某台机器的本机路径"，在本机
			打开时指的就是同一个本机路径。

			0.18.0 之前的项目把项目内路径记作不带 \c{./} 的相对路径，这种文本同样
			按项目文件夹解析，保证旧项目仍然指向原来的位置。
		*/
		YSSURI strToUri(const QString& text) const {
			if (text.isEmpty()) {
				return YSSURI();
			}
			if (text.startsWith(RemoteLocalPrefix)) {
				return YSSURI::fromLocalPath(text.mid(RemoteLocalPrefix.size()));
			}
			if (text.startsWith(QStringLiteral("./"))) {
				const YSSURI folder = projectFolder();
				if (not folder.isValid()) {
					return YSSURI();
				}
				const QString relative = text.mid(2);
				return relative.isEmpty() ? folder : folder.resolve(relative);
			}
			const YSSURI parsed = YSSURI::fromString(text);
			// 0.18.0 之前项目内路径记作不带 "./" 的相对路径，这里按项目文件夹解析，
			// 保证旧项目打开后图标、打开文件列表等仍然指向原来的位置。
			if (parsed.isLocal() and not QFileInfo(text).isAbsolute()) {
				const YSSURI folder = projectFolder();
				if (folder.isValid()) {
					return folder.resolve(text);
				}
			}
			return parsed;
		}

		/*
			读取一个路径列表节点，其中每项都是 \l{uriToStr()} 的产物。
		*/
		QList<YSSURI> readUriList(const QString& key) const {
			QList<YSSURI> files;
			const QStringList keys = ProjectConfig.keys(key);
			for (const QString& itemKey : keys) {
				const YSSURI uri = strToUri(ProjectConfig.getString(key + "." + itemKey));
				if (uri.isValid()) {
					files.append(uri);
				}
			}
			return files;
		}

		/*
			将路径中的 "." 替换为 "|"（Windows 路径非法字符），
			避免 JsonConfig 的 "." 键分隔符误切路径段。
			由于 "|" 不会出现在任何合法项目路径中，转换完全可逆。
		*/
		static QString escapeDots(const QString& path) {
			QString result = path;
			result.replace(QLatin1Char('.'), QLatin1Char('|'));
			return result;
		}

		/*
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

		/*
			构造回收站虚拟路径：同备份路径规则。
		*/
		static QString trashVirtualPath(const QString& inProjRelativePth,
			const QDateTime& time,
			const QString& extension) {
			return backupVirtualPath(inProjRelativePth, time, extension);
		}

		/*
			从备份虚拟路径的文件名中解析出 QDateTime。
			文件名格式：yyyyMMdd_hh_mm_ss.ext
		*/
		static QDateTime parseTimestampFromName(const QString& fileName) {
			// 取第一个 '.' 之前的部分尝试解析
			int dotPos = fileName.indexOf(QLatin1Char('.'));
			QString tsPart = (dotPos >= 0) ? fileName.left(dotPos) : fileName;
			return QDateTime::fromString(tsPart, "yyyyMMdd_hh_mm_ss");
		}

		/*
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

	const QString YSSProjectPrivate::RemoteLocalPrefix = QStringLiteral("remote:///");

	const Visindigo::General::Version YSSProjectPrivate::ProjectVersion = Visindigo::General::Version(0, 2, 0);
	const QMap<Visindigo::General::Version, std::tuple<Visindigo::General::Version, Visindigo::General::Version>> YSSProjectPrivate::ProjectVersionRange = {
		{ Visindigo::General::Version(0, 1, 0), { Visindigo::General::Version(0, 13, 0), Visindigo::General::Version(0, 16, 3) } },
		{ Visindigo::General::Version(0, 2, 0), { Visindigo::General::Version(0, 17, 0), Visindigo::General::Version(0, 0, 0) } }
	};
	/*!
		\class YSSCore::General::YSSProject
		\brief 这个类代表YSS项目数据.
		\since YSS 0.13.0
		\inmodule YSSCore

		YSSProject类代表YSS项目数据。它是作为project.yssp文件的API操作以及读写使用，只负责数据操作，
		不负责在程序中充当一个关于项目各项内容更改时的数据集散中心。

		自YSS 0.18.0起，项目位置由 \l{YSSURI} 标识，因此本地项目与SSH远端项目使用同一套API。
		YSSProject是QObject派生类，因为读写都是异步的：\l{loadProject()} 不会同步返回结果，
		调用方需要连接 \l{projectLoaded()} / \l{projectLoadFailed()} 获取结论；
		\l{saveProject()} 同理，写盘期间 \l{isSaving()} 为 true，结束后由
		\l{projectSaved()} / \l{projectSaveFailed()} 通知。

		譬如，如果调用此类的setFocusedFile函数，它只会修改对应yssp文件中记录的数据，
		不会真的使YSS编辑器变更当前打开的文件。这个需求需要用户调用FileServerManager
		的changeFocusedFile信号进行。稍后，YSS会根据是否实际变更了打开的文件来读写YSSP。

		YSSProject不是单例类，你可以根据需要在程序各个地方创建
		任意多的YSSProject示例，但YSSProject语义上只允许一个YSSProject作为
		“当前项目”，你可以通过getCurrentProject和setCurrentProject读写它。当前项目
		可以为nullptr，代表当前没有打开任何项目。

		要从本类直接初始化一个YSS项目，请参见initConfig函数的说明。
	*/

	/*!
		\enum YSSCore::General::YSSProject::LoadError
		\since YSS 0.13.0

		\value None 没有错误。加载成功后 \l{getLoadError()} 返回此值。
		\value Unknown 未知的加载结果
		\value InvalidUri 传入的配置路径不是合法的URI
		\value NotFound 配置文件不存在
		\value ReadFailed 配置文件存在，但读取失败（权限、连接断开等）
		\value ParseError 项目的JSON格式有误
		\value InvalidConfig 项目的JSON格式无误，但配置无效
	*/

	/*!
		\since YSS 0.17.0

		返回当前YSS版本下，预设的项目版本号。
	*/
	Visindigo::General::Version YSSProject::getCurrentProjectVersion() {
		return YSSProjectPrivate::ProjectVersion;
	}

	/*!
		\since YSS 0.17.0
		返回YSS项目版本兼容范围。

		如果返回的tuple中，第二个Version为0.0.0，代表没有上限。
	*/
	QMap<Visindigo::General::Version, std::tuple<Visindigo::General::Version, Visindigo::General::Version>> YSSProject::getProjectVersionRange() {
		return YSSProjectPrivate::ProjectVersionRange;
	}

	/*!
		\since YSS 0.13.0
		构造函数，创建一个空的项目
	*/
	YSSProject::YSSProject(QObject* parent) : QObject(parent) {
		d = new YSSProjectPrivate();
	}

	/*!
		\since YSS 0.13.0
		析构函数。

		析构时若仍有保存请求在途，写入本身不会被打断（数据已交给 UniqueFileSystem），
		只是 \l{projectSaved()} / \l{projectSaveFailed()} 不会再发给这个对象。
	*/
	YSSProject::~YSSProject() {
		if (YSSProjectPrivate::CurrentProject == this) {
			YSSProjectPrivate::CurrentProject = nullptr;
		}
		delete d;
	}

	/*!
		\since YSS 0.13.0
		从 \a configPath 异步加载YSS项目。可以传入任意 scheme 的URI：本地文件、
		SSH远端文件或Qt资源。

		本函数立即返回，结果通过信号通知：
		\list
		\li 成功时发出 \l{projectLoaded()}，此时 \l{getProjectPath()} 为 \a configPath；
		\li 失败时发出 \l{projectLoadFailed()}，参数给出原因。
		\endlist

		失败时 \l{isLoaded()} 为 false，且不会修改已有的 \l{getProjectPath()}。
	*/
	void YSSProject::loadProject(const YSSURI& configPath) {
		if (not configPath.isValid()) {
			yErrorF << "Invalid project config uri:" << configPath.toString();
			d->Loaded = false;
			d->LastLoadError = LoadError::InvalidUri;
			emit projectLoadFailed(d->LastLoadError);
			return;
		}
		YSSFileSystemReply* reply = YSSUniqueFileSystem::getInstance()->read(configPath);
		if (reply == nullptr) {
			yErrorF << "Failed to request project config: " << configPath.toString();
			d->Loaded = false;
			d->LastLoadError = LoadError::ReadFailed;
			emit projectLoadFailed(d->LastLoadError);
			return;
		}
		connect(reply, &YSSFileSystemReply::finished, this, [this, configPath, reply]() {
			const YSSFileError fileError = reply->getError();
			if (fileError.isError()) {
				d->Loaded = false;
				d->LastLoadError = (fileError.Type == YSSFileError::ErrorType::NotFound)
					? LoadError::NotFound : LoadError::ReadFailed;
				yErrorF << "Failed to read project config: " << configPath.toString()
					<< ", error: " << fileError.toString();
				emit projectLoadFailed(d->LastLoadError);
				return;
			}
			const QByteArray data = reply->getData();
			if (d->ProjectConfig.parse(QString::fromUtf8(data)).error != QJsonParseError::NoError) {
				yErrorF << "Failed to parse project config: " << configPath.toString();
				d->Loaded = false;
				d->LastLoadError = LoadError::ParseError;
				emit projectLoadFailed(d->LastLoadError);
				return;
			}
			const QStringList crucialKeys = {
				"Project.Name",
				"Project.CreateTime",
				"Project.LastModifyTime",
				"Project.Version",
				"Project.DebugServerID",
				"Project.Description",
				"Project.IconPath"
			};
			for (const QString& key : crucialKeys) {
				if (!d->ProjectConfig.contains(key)) {
					yErrorF << "Project config missing crucial key: " << key
						<< ", config: " << configPath.toString();
					d->Loaded = false;
					d->LastLoadError = LoadError::InvalidConfig;
					emit projectLoadFailed(d->LastLoadError);
					return;
				}
			}
			d->ConfigPath = configPath;
			d->Loaded = true;
			d->LastLoadError = LoadError::None;
			emit projectLoaded();
		});
	}

	/*!
		\since YSS 0.13.0
		异步保存项目。

		\a configPath 默认为无效URI，代表在加载位置原地保存。如果传入其它值，
		就相当于另存为，之后的保存都会写到这里。

		本函数立即返回。写盘期间 \l{isSaving()} 为 true；若在写入结束前再次调用，
		不会重复排队，而是在当前写入结束后按最新内容再写一次。
		写盘结束发出 \l{projectSaved()} 或 \l{projectSaveFailed()}。
	*/
	void YSSProject::saveProject(const YSSURI& configPath) {
		if (configPath.isValid()) {
			d->ConfigPath = configPath;
		}
		if (not d->ConfigPath.isValid()) {
			yErrorF << "Project config path is not set, cannot save project";
			emit projectSaveFailed();
			return;
		}
		if (d->Saving) {
			d->SavePending = true;
			return;
		}
		d->Saving = true;
		d->SavePending = false;
		d->updateLastModifyTime();
		const QByteArray content = d->ProjectConfig.toString().toUtf8();
		YSSFileSystemReply* reply = YSSUniqueFileSystem::getInstance()->write(d->ConfigPath, content);
		if (reply == nullptr) {
			yErrorF << "Failed to request project config write: " << d->ConfigPath.toString();
			d->Saving = false;
			emit projectSaveFailed();
			return;
		}
		connect(reply, &YSSFileSystemReply::finished, this, [this, reply]() {
			const YSSFileError fileError = reply->getError();
			const bool ok = not fileError.isError();
			if (not ok) {
				yErrorF << "Failed to save project config: " << d->ConfigPath.toString()
					<< ", error: " << fileError.toString();
			}
			d->Saving = false;
			if (ok) {
				emit projectSaved();
			}
			else {
				emit projectSaveFailed();
			}
			if (d->SavePending) {
				d->SavePending = false;
				saveProject();
			}
		});
	}

	/*!
		\since YSS 0.13.0
		return 是否已加载（含由 \l{initConfig()} 初始化）了项目配置。

		这里只反映最近一次加载的结果；要拿到失败原因请用 \l{getLoadError()}。
	*/
	bool YSSProject::isLoaded() const {
		return d->Loaded;
	}

	/*!
		\since YSS 0.18.0
		return 是否有正在进行的项目保存。
	*/
	bool YSSProject::isSaving() const {
		return d->Saving;
	}

	/*!
		\since YSS 0.18.0
		return 最近一次 \l{loadProject()} 的结果；成功时为 \l{LoadError::None}。
	*/
	YSSProject::LoadError YSSProject::getLoadError() const {
		return d->LastLoadError;
	}

	/*!
		\since YSS 0.18.0
		在 \a folder 指定的文件夹初始化项目配置，名称为 \a name

		本函数只在内存里准备项目配置，并把配置路径设为 \c{folder/project.yssp}：
		不创建文件夹、不检查文件夹是否为空、也不写盘。

		\note 文件夹的创建与所有文件的写入都由调用方通过 \l{YSSUniqueFileSystem} 完成，
		这样本地项目与远端项目走同一条路径。
		\note 初始化完成后调用方需要自行调用 \l{saveProject()} 把配置写到目标位置。

		return 是否成功初始化项目配置
	*/
	bool YSSProject::initConfig(const YSSURI& folder, const QString& name) {
		if (not folder.isValid()) {
			yErrorF << "Cannot init project config with invalid folder URI";
			return false;
		}
		d->ConfigPath = folder.resolve("project.yssp");
		d->ProjectConfig.setString("Project.Name", name);
		d->ProjectConfig.setString("Project.Description", "");
		d->ProjectConfig.setString("Project.IconPath", "");
		d->ProjectConfig.setString("Project.Version", getCurrentProjectVersion().toString());
		d->ProjectConfig.setString("Project.Author", "");
		d->ProjectConfig.setString("Project.DebugServerID", "");
		d->ProjectConfig.setString("Project.CreateTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		d->ProjectConfig.setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		d->ProjectConfig.setStringList("Project.RequiredPlugins", QStringList());
		refreshLastModifyTime();
		d->Loaded = true;
		d->LastLoadError = LoadError::None;
		return true;
	}

	/*!
		\since YSS 0.18.0
		return 整份项目配置的副本。

		项目模板用它把准备好的配置导出给调用方，调用方再通过 \l{setYSSProjectContent()}
		设置到新建的 YSSProject 上。本函数不写盘。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getYSSProjectContent() const {
		return d->ProjectConfig;
	}

	/*!
		\since YSS 0.18.0
		\a config 整份项目配置

		用 \a config 替换当前的项目配置。本函数不写盘，需要落盘时自行调用 \l{saveProject()}。
	*/
	void YSSProject::setYSSProjectContent(const Visindigo::Utility::JsonConfig& config) {
		d->ProjectConfig = config;
	}

	/*!
		\since YSS 0.13.0
		获得项目名称
	*/
	QString YSSProject::getProjectName() {
		return d->ProjectConfig.getString("Project.Name");
	}

	/*!
		\since YSS 0.13.0
		获得项目描述
	*/
	QString YSSProject::getProjectDescription() {
		return d->ProjectConfig.getString("Project.Description");
	}

	/*!
		\since YSS 0.13.0
		return 当前项目的文件夹路径。

		项目文件夹就是配置文件所在目录。返回的URI跟随项目位置：本地项目是本地目录，
		远端项目是远端主机上的目录。需要本地路径时请用 \l{YSSURI::getLocalPath()}。
	*/
	YSSURI YSSProject::getProjectFolder() {
		return d->projectFolder();
	}

	/*!
		\since YSS 0.17.0

		return 当前项目的缓存文件夹路径。

		缓存文件夹固定为项目文件夹下的 \c{.yss} 目录，用于存放项目自身的各种临时数据与缓存。
		它可能不存在（例如由更早版本创建的项目），此时调用方应当自行建立它。
	*/
	YSSURI YSSProject::getProjectCacheFolder() {
		return d->yssDir();
	}

	/*!
		\since YSS 0.13.0
		return 当前项目的配置文件路径。

		如果项目加载时是有效的，则返回项目配置文件的URI，否则返回无效URI。
	*/
	YSSURI YSSProject::getProjectPath() {
		return d->ConfigPath;
	}

	/*!
		\since YSS 0.13.0
		return 当前项目的图标路径。

		这个图标路径忠实返回项目文件设置的值，不管他
		是否真的有效，以及是什么格式的路径。
	*/
	YSSURI YSSProject::getProjectIconPath() {
		return d->strToUri(d->ProjectConfig.getString("Project.IconPath"));
	}

	/*!
		\since YSS 0.13.0
		return 项目作者设置。
	*/
	QString YSSProject::getProjectAuthor() {
		return d->ProjectConfig.getString("Project.Author");
	}

	/*!
		\since YSS 0.13.0
		设置项目名称为 \a name
	*/
	void YSSProject::setProjectName(const QString& name) {
		d->ProjectConfig.setString("Project.Name", name);
	}

	/*!
		\since YSS 0.13.0
		设置项目描述为 \a description
	*/
	void YSSProject::setProjectDescription(const QString& description) {
		d->ProjectConfig.setString("Project.Description", description);
	}

	/*!
		\since YSS 0.13.0
		设置项目图标路径为 \a iconPath

		路径按 \l{uriToStr()} 的规则记录：落在项目文件夹内的图标存成相对路径，
		因此项目整体搬迁后依然有效。
	*/
	void YSSProject::setProjectIconPath(const YSSURI& iconPath) {
		d->ProjectConfig.setString("Project.IconPath", d->uriToStr(iconPath));
	}

	/*!
		\since YSS 0.13.0
		设置项目作者为 \a author
	*/
	void YSSProject::setProjectAuthor(const QString& author) {
		d->ProjectConfig.setString("Project.Author", author);
	}

	/*!
		\since YSS 0.13.0
		return 项目创建时间
	*/
	QDateTime YSSProject::getProjectCreateTime() {
		return QDateTime::fromString(d->ProjectConfig.getString("Project.CreateTime"), "yyyy-MM-dd hh:mm:ss");
	}

	/*!
		\since YSS 0.13.0
		return 项目上次修改时间
	*/
	QDateTime YSSProject::getProjectLastModifyTime() {
		return QDateTime::fromString(d->ProjectConfig.getString("Project.LastModifyTime"), "yyyy-MM-dd hh:mm:ss");
	}

	/*!
		\since YSS 0.13.0
		return 项目设置的调试服务器ID。

		由于YSS项目目前假定只面向那些编码项目，而编码项目
		或多或少需要调试功能，因此提供了这个API。
	*/
	QString YSSProject::getProjectDebugServerID() {
		return d->ProjectConfig.getString("Project.DebugServerID");
	}

	/*!
		\since YSS 0.13.0
		设置项目的调试服务器ID为 \a id

		由于YSS项目目前假定只面向那些编码项目，而编码项目
		或多或少需要调试功能，因此提供了这个API。
	*/
	void YSSProject::setProjectDebugServerID(const QString& id) {
		d->ProjectConfig.setString("Project.DebugServerID", id);
		emit YSSCore::Editor::DebugServerManager::getInstance()->projectDebugServerChanged(id);
	}

	/*!
		\since YSS 0.13.0
		return 项目的版本号。从initConfig初始化时，默认初始化为0.1.
	*/
	Visindigo::General::Version YSSProject::getProjectVersion() {
		return Visindigo::General::Version(d->ProjectConfig.getString("Project.Version"));
	}

	/*!
		\since YSS 0.13.0
		return 项目中为指定插件 \a plugin 保存的配置数据。

		请注意，获取配置数据并修改后，需要通过saveProjectConfigForPlugin重新设置到项目里。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getProjectConfigForPlugin(Visindigo::General::Plugin* plugin) {
		QString key = "MetaData." + plugin->getPluginID();
		return d->ProjectConfig.getObject(key);
	}

	/*!
		\since YSS 0.13.0
		return 项目中为指定插件 \a pluginID 保存的配置数据。这个重载是按插件ID而非指针索引的。

		请注意，获取配置数据并修改后，需要通过saveProjectConfigForPlugin重新设置到项目里。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getProjectConfigForPlugin(const QString& pluginID) {
		QString key = "MetaData." + pluginID;
		return d->ProjectConfig.getObject(key);
	}

	/*!
		\since YSS 0.17.0

		return 项目中为树形布局保存的配置数据。这个配置数据是一个JsonConfig对象，包含了树形布局的相关设置。
	*/
	Visindigo::Utility::JsonConfig YSSProject::getTreeLayoutConfig() {
		return d->ProjectConfig.getObject("TreeLayout");
	}

	/*!
		\since YSS 0.17.0
		为树形布局设置配置数据。这个配置数据是一个JsonConfig对象，包含了树形布局的相关设置。

		\a config 树形布局配置数据。
	*/
	void YSSProject::setTreeLayoutConfig(const Visindigo::Utility::JsonConfig& config) {
		d->ProjectConfig.setObject("TreeLayout", config);
	}

	/*!
		\since YSS 0.17.0

		return 项目中是否有树形布局的配置数据。
	*/
	bool YSSProject::hasTreeLayoutConfig() {
		return d->ProjectConfig.contains("TreeLayout");
	}

	/*!
		\since YSS 0.13.0
		为指定插件 \a plugin 保存具有 \a config 的数据。

		它被存储在YSSP文件的MetaData节点下。
	*/
	void YSSProject::saveProjectConfigForPlugin(Visindigo::General::Plugin* plugin, const Visindigo::Utility::JsonConfig& config) {
		QString key = "MetaData." + plugin->getPluginID();
		d->ProjectConfig.setObject(key, config);
	}

	/*!
		\since YSS 0.13.0
		为指定插件 \a pluginID 保存具有 \a config 的数据。这个重载是按插件ID而非指针索引的。

		它被存储在YSSP文件的MetaData节点下。
	*/
	void YSSProject::saveProjectConfigForPlugin(const QString& pluginID, const Visindigo::Utility::JsonConfig& config) {
		QString key = "MetaData." + pluginID;
		d->ProjectConfig.setObject(key, config);
	}

	/*!
		\since YSS 0.13.0

		刷新项目的最后修改时间。它被直接刷新为调用此函数的时间。
	*/
	void YSSProject::refreshLastModifyTime() {
		d->ProjectConfig.setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	}

	/*!
		\since YSS 0.13.0

		return 项目中编辑器当前打开的文件列表。本机文件、远端文件、虚拟文档、
		Qt 资源都以 \l{YSSURI} 返回，不再压平成字符串。

		这个函数不区分区域，它会自动检索所有区域的打开文件列表，返回所有文件。

		如果需要区分区域的版本，请使用getEditorOpenedFilesInArea函数。
	*/
	QList<YSSURI> YSSProject::getEditorOpenedFiles() {
		QList<YSSURI> files = d->readUriList("Editor.OpenedFiles");
		const QStringList extraKeys = d->ProjectConfig.keys("Editor.OpenedFilesExtra");
		for (const QString& areaID : extraKeys) {
			files.append(d->readUriList("Editor.OpenedFilesExtra." + areaID));
		}
		return files;
	}

	/*!
		\since YSS 0.17.0

		return 项目中编辑器当前打开的文件列表，按区域区分。元素为 \l{YSSURI}。
	*/
	QMap<QString, QList<YSSURI>> YSSProject::getEditorOpenedFilesInArea() {
		QMap<QString, QList<YSSURI>> areaFiles;
		areaFiles["0"] = d->readUriList("Editor.OpenedFiles");
		QStringList extraKeys = d->ProjectConfig.keys("Editor.OpenedFilesExtra");
		for (const QString& areaID : extraKeys) {
			QString realID = areaID;
			realID.remove(0, 2); // 去掉前缀 "s_"
			areaFiles[realID] = d->readUriList("Editor.OpenedFilesExtra." + areaID);
		}
		return areaFiles;
	}

	/*!
		\since YSS 0.17.0
		\a uri 编辑器打开的文件，可以是本机、远端、虚拟文档或 Qt 资源。
		\a areaID 编辑器打开文件所在的区域ID，默认为空字符串。

		向项目中添加一个编辑器打开的文件。如果这个文件落在项目文件夹内，则
		转换为相对路径存储，否则以绝对形式存储；具体规则见 \l{uriToStr()}。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被打开了，但它并不会真的使编辑器打开这个文件。

		\note 在0.17.0支持多窗口之前，没有areaID参数。自0.17.0起，编辑器可能有多个窗口，每个窗口都有自己的打开文件列表。
		为了兼容考虑，如果areaID为空或"0"，则表示默认的主窗口，且继续保存在Editor.OpenedFiles节点下，如果areaID不为空且不为"0"，
		则表示其他窗口，保存到Editor.OpenedFilesExtra.<areaID>节点下。
	*/
	void YSSProject::addEditorOpenedFile(const YSSURI& uri, const QString& areaID) {
		if (not uri.isValid()) {
			yErrorF << "Invalid URI, cannot add to project opened files";
			return;
		}
		const QList<YSSURI> files = getEditorOpenedFiles();
		if (files.contains(uri)) {
			return;
		}
		const QString storedPath = d->uriToStr(uri);
		if (areaID.isEmpty() || areaID == "0") {
			d->ProjectConfig.setString("Editor.OpenedFiles." + QString::number(d->ProjectConfig.keys("Editor.OpenedFiles").size()), storedPath);
		}
		else {
			d->ProjectConfig.setString("Editor.OpenedFilesExtra.s_" + areaID + "." + QString::number(d->ProjectConfig.keys("Editor.OpenedFilesExtra.s_" + areaID).size()), storedPath);
		}
	}

	/*!
		\since YSS 0.17.0
		\a uri 编辑器打开的文件。
		\a fromID 编辑器打开文件所在的原区域ID，默认为空字符串。
		\a toID 编辑器打开文件所在的新区域ID，默认为空字符串。

		将项目中一个编辑器打开的文件从一个区域移动到另一个区域。如果fromID中没有此文件，则不会移动。
		如果fromID和toID相同，不做任何操作。
	*/
	void YSSProject::moveEditorOpenedFile(const YSSURI& uri, const QString& fromID, const QString& toID) {
		if (fromID == toID) {
			return;
		}
		if (not getEditorOpenedFiles().contains(uri)) {
			return;
		}
		removeEditorOpenedFile(uri);
		addEditorOpenedFile(uri, toID);
	}
	/*!
		\since YSS 0.13.0
		\a uris 编辑器打开的文件列表。

		有关其中每个文件的说明，请参见addEditorOpenedFile函数。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这些文件被打开了，但它并不会真的使编辑器打开这些文件。

		这个函数将把所有文件都添加到默认区域。这个函数没有区分区域的能力。若要区分区域，请用setEditorOpenedFilesInArea函数。
	*/
	void YSSProject::setEditorOpenedFiles(const QList<YSSURI>& uris) {
		removeAllEditorOpenedFiles();
		for (const YSSURI& uri : uris) {
			addEditorOpenedFile(uri);
		}
	}

	/*!
		\since YSS 0.17.0
		\a urisInArea 编辑器打开的文件列表，按区域区分。

		有关其中每个文件的说明，请参见addEditorOpenedFile函数。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这些文件被打开了，但它并不会真的使编辑器打开这些文件。
	*/
	void YSSProject::setEditorOpenedFilesInArea(const QMap<QString, QList<YSSURI>>& urisInArea) {
		removeAllEditorOpenedFiles();
		for (auto key : urisInArea.keys()) {
			for (const YSSURI& uri : urisInArea[key]) {
				addEditorOpenedFile(uri, key);
			}
		}
	}

	/*!
		\since YSS 0.13.0
		\a uri 编辑器打开的文件。

		移除项目中一个编辑器打开的文件。

		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被关闭了，但它并不会真的使编辑器关闭这个文件。

		这个函数不区分区域，它会自动检索所有区域的打开文件列表，找到匹配的文件后移除。
	*/
	void YSSProject::removeEditorOpenedFile(const YSSURI& uri) {
		if (not getEditorOpenedFiles().contains(uri)) {
			return;
		}
		QString areaID = getFileAreaID(uri);
		if (areaID.isEmpty() || areaID == "0") {
			QStringList keys = d->ProjectConfig.keys("Editor.OpenedFiles");
			for (const QString& key : keys) {
				const YSSURI fileUri = d->strToUri(d->ProjectConfig.getString("Editor.OpenedFiles." + key));
				if (fileUri == uri) {
					d->ProjectConfig.remove("Editor.OpenedFiles." + key);
					break;
				}
			}
		}
		else {
			QStringList keys = d->ProjectConfig.keys("Editor.OpenedFilesExtra.s_" + areaID);
			for (const QString& key : keys) {
				const YSSURI fileUri = d->strToUri(d->ProjectConfig.getString("Editor.OpenedFilesExtra.s_" + areaID + "." + key));
				if (fileUri == uri) {
					d->ProjectConfig.remove("Editor.OpenedFilesExtra.s_" + areaID + "." + key);
					break;
				}
			}
		}
	}

	/*!
		\since YSS 0.13.0
		\a uri 编辑器聚焦的文件。

		设置项目中编辑器当前聚焦的文件。传入无效URI表示清空。

		这个API和YSSCore::Editor::FileServerManager::focusFile信号无关
		它只是单纯地修改项目文件中的数据，告诉项目这个文件被聚焦了，但它并不会真的使编辑器聚焦这个文件。

		如果你希望YSS更改聚焦的文件，应该使用上面提到的这个信号。
	*/
	void YSSProject::setFocusedFile(const YSSURI& uri) {
		if (not uri.isValid()) {
			d->ProjectConfig.setString("Editor.FocusedFile", "");
			return;
		}
		// 虚拟文档没有文件系统实体，只要求其余类型确实存在。
		if (not uri.isVirtual() and not uri.isFile()) {
			yErrorF << "Focused file does not exist:" << uri.toString();
			return;
		}
		d->ProjectConfig.setString("Editor.FocusedFile", d->uriToStr(uri));
	}

	/*!
		\since YSS 0.17.0

		\a uri 文件。

		return 指定文件所在的区域ID，如果文件不在任何区域中打开，则返回空字符串。
	*/
	QString YSSProject::getFileAreaID(const YSSURI& uri) {
		if (not uri.isValid()) {
			return QString();
		}
		if (d->ProjectConfig.contains("Editor.OpenedFiles")) {
			QStringList keys = d->ProjectConfig.keys("Editor.OpenedFiles");
			for (const QString& key : keys) {
				const YSSURI fileUri = d->strToUri(d->ProjectConfig.getString("Editor.OpenedFiles." + key));
				if (fileUri == uri) {
					return "0"; // 默认区域
				}
			}
		}
		if (d->ProjectConfig.contains("Editor.OpenedFilesExtra")) {
			QStringList areaKeys = d->ProjectConfig.keys("Editor.OpenedFilesExtra");
			for (const QString& areaID : areaKeys) {
				QStringList fileKeys = d->ProjectConfig.keys("Editor.OpenedFilesExtra." + areaID);
				for (const QString& key : fileKeys) {
					const YSSURI fileUri = d->strToUri(d->ProjectConfig.getString("Editor.OpenedFilesExtra." + areaID + "." + key));
					if (fileUri == uri) {
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

		return 项目中编辑器当前聚焦的文件。没有聚焦文件时返回无效URI。
	*/
	YSSURI YSSProject::getFocusedFile() {
		return d->strToUri(d->ProjectConfig.getString("Editor.FocusedFile"));
	}

	/*!
		\since YSS 0.13.0

		return 项目中编辑器当前聚焦的文件的文件名（不带路径）。对于虚拟文件路径，返回虚拟文件路径的名称部分。对于真实文件路径，返回真实文件的文件名。
	*/
	QString YSSProject::getFocusedFileName() {
		return getFocusedFile().getFileName();
	}

	/*!
		\since YSS 0.13.0
		从项目中移除所有编辑器打开的文件路径。
		这个API和YSSCore::Editor::FileServerManager中的编辑器打开文件列表是分离的。
		它只是单纯地修改项目文件中的数据，告诉项目没有文件被打开了，但它并不会真的使编辑器关闭所有文件。
	*/
	void YSSProject::removeAllEditorOpenedFiles() {
		d->ProjectConfig.remove("Editor.OpenedFiles");
		d->ProjectConfig.remove("Editor.OpenedFilesExtra");
	}

	/*!
		\since YSS 0.17.0
		\a areaID 编辑器打开文件所在的区域ID，默认为空字符串。

		从项目中移除指定区域的所有编辑器打开的文件路径。
	*/
	void YSSProject::removeEditorOpenedFilesInArea(const QString& areaID) {
		if (areaID.isEmpty() || areaID == "0") {
			d->ProjectConfig.remove("Editor.OpenedFiles");
		}
		else {
			d->ProjectConfig.remove("Editor.OpenedFilesExtra.s_" + areaID);
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
		\since YSS 0.16.0
		为项目内相对路径 \a inProjRelativePth 指向的文件创建一份备份。
		文件内容从磁盘按原路径读取，存入备份虚拟存储中。
	*/
	void YSSProject::createFileBackup(const QString& inProjRelativePth) {
		Visindigo::Utility::VirtualStorage* storage = d->getBackupStorage();
		if (storage == nullptr) {
			return;
		}
		QString absPath = getProjectFolder().resolve(inProjRelativePth).getLocalPath();
		if (absPath.isEmpty()) {
			yErrorF << "Project file backup only supports local projects, current path: "
				<< getProjectPath().toString();
			return;
		}
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

		storage->saveFile(vpth, content);

		// 清理超出上限的旧备份
		QString folder = d->escapeDots(inProjRelativePth);
		Visindigo::Utility::JsonConfig listing = storage->listFolder(folder, false);
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
				storage->removeFile(folder + "/" + sorted[i].second);
			}
		}
	}

	/*!
		\since YSS 0.16.0
		返回 \a inProjRelativePth 的所有备份时间列表。
	*/
	QList<QDateTime> YSSProject::getFileBackupList(const QString& inProjRelativePth) {
		QList<QDateTime> result;
		Visindigo::Utility::VirtualStorage* storage = d->getBackupStorage();
		if (storage == nullptr) {
			return result;
		}
		QString folder = d->escapeDots(inProjRelativePth);
		Visindigo::Utility::JsonConfig listing = storage->listFolder(folder, false);
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
		\since YSS 0.16.0
		获取 \a inProjRelativePth 在 \a backupTime 时的备份内容。
	*/
	QByteArray YSSProject::getFileBackupContent(const QString& inProjRelativePth,
	                                             const QDateTime& backupTime) {
		Visindigo::Utility::VirtualStorage* storage = d->getBackupStorage();
		if (storage == nullptr) {
			return QByteArray();
		}
		QString ext = d->fileExtension(inProjRelativePth);
		QString vpth = d->backupVirtualPath(inProjRelativePth, backupTime, ext);
		return storage->readFile(vpth);
	}

	/*!
		\since YSS 0.16.0
		将 \a inProjRelativePth 在 \a backupTime 的备份还原到磁盘。
		若 \a restoreToPath 非空，则还原到指定路径；
		若 \a overwrite 为 false 且目标已存在，则不覆盖返回 false。

		return 还原成功返回 true；目标已存在且不覆盖时返回 false。
	*/
	bool YSSProject::restoreFileBackup(const QString& inProjRelativePth,
	                                    const QDateTime& backupTime,
	                                    const QString& restoreToPath, bool overwrite) {
		Visindigo::Utility::VirtualStorage* storage = d->getBackupStorage();
		if (storage == nullptr) {
			return false;
		}
		QString ext = d->fileExtension(inProjRelativePth);
		QString vpth = d->backupVirtualPath(inProjRelativePth, backupTime, ext);
		QByteArray content = storage->readFile(vpth);
		if (content.isEmpty()) {
			return false;
		}

		QString targetPath = restoreToPath.isEmpty()
			? getProjectFolder().resolve(inProjRelativePth).getLocalPath()
			: restoreToPath;
		if (targetPath.isEmpty()) {
			yErrorF << "Project file backup only supports local projects, current path: "
				<< getProjectPath().toString();
			return false;
		}

		if (!overwrite && Visindigo::Utility::FileUtility::isFileExist(targetPath)) {
			return false;
		}

		Visindigo::Utility::FileOperation::ErrorCode saveResult = Visindigo::Utility::FileOperation::saveBinary(targetPath, content);
		if (saveResult != Visindigo::Utility::FileOperation::Success) {
			yErrorF << "Failed to restore file from backup: " << targetPath
				<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(saveResult);
			return false;
		}
		return true;
	}

	/*!
		\since YSS 0.16.0
		删除 \a inProjRelativePth 在 \a backupTime 的单份备份。
	*/
	void YSSProject::removeFileBackup(const QString& inProjRelativePth,
	                                   const QDateTime& backupTime) {
		Visindigo::Utility::VirtualStorage* storage = d->getBackupStorage();
		if (storage == nullptr) {
			return;
		}
		QString ext = d->fileExtension(inProjRelativePth);
		QString vpth = d->backupVirtualPath(inProjRelativePth, backupTime, ext);
		storage->removeFile(vpth);
	}

	/*!
		\since YSS 0.16.0
		删除 \a inProjRelativePth 的所有备份。
	*/
	void YSSProject::removeFileBackup(const QString& inProjRelativePth) {
		Visindigo::Utility::VirtualStorage* storage = d->getBackupStorage();
		if (storage == nullptr) {
			return;
		}
		QString folder = d->escapeDots(inProjRelativePth);
		Visindigo::Utility::JsonConfig listing = storage->listFolder(folder, false);
		Visindigo::Utility::JsonConfig filesObj = listing.getObject("files");
		QStringList fileNames = filesObj.keys();
		for (const QString& name : fileNames) {
			storage->removeFile(folder + "/" + name);
		}
	}

	/*!
		\since YSS 0.16.0
		删除项目中所有文件的全部备份。
		直接销毁数据库文件并重建空库，避免逐条 SQL DELETE 的开销。
	*/
	void YSSProject::removeAllFileBackup() {
		d->resetStorage(d->BackupStorage, "backup.db");
	}

	/*!
		\since YSS 0.16.0
		return 所有已备份文件的原始项目内相对路径列表。
	*/
	QStringList YSSProject::getAllFileBackups() {
		QStringList result;
		Visindigo::Utility::VirtualStorage* storage = d->getBackupStorage();
		if (storage == nullptr) {
			return result;
		}
		Visindigo::Utility::JsonConfig root = storage->listFolder("", false);
		Visindigo::Utility::JsonConfig foldersObj = root.getObject("sub_folders");
		QStringList escapedNames = foldersObj.keys();
		for (const QString& escaped : escapedNames) {
			QString origPath = escaped;
			origPath.replace(QLatin1Char('|'), QLatin1Char('.'));
			result.append(origPath);
		}
		return result;
	}

	/*!
		\since YSS 0.16.0
		设置每个文件保留的最大备份数量。超过上限时，最旧的备份会被自动清理。

		\a count 保留的最大备份数量。
	*/
	void YSSProject::setFileBackupMaxCount(qint32 count) {
		d->BackupMaxCount = (count > 0) ? count : 1;
	}

	/*!
		\since YSS 0.16.0

		return 当前设置的每文件最大备份数量。
	*/
	qint32 YSSProject::getFileBackupMaxCount() {
		return d->BackupMaxCount;
	}

	/*!
		\since YSS 0.17.0

		添加一个项目所需的插件ID到项目配置中。如果插件ID已存在，则不会重复添加。
	*/
	void YSSProject::addRequiredPlugin(const QString& pluginID) {
		QStringList requiredPlugins = d->ProjectConfig.getStringList("Project.RequiredPlugins");
		if (not requiredPlugins.contains(pluginID)) {
			requiredPlugins.append(pluginID);
			d->ProjectConfig.setStringList("Project.RequiredPlugins", requiredPlugins);
		}
	}

	/*!
		\since YSS 0.17.0
		移除一个项目所需的插件ID从项目配置中。如果插件ID不存在，则不会有任何操作。
	*/
	void YSSProject::removeRequiredPlugin(const QString& pluginID) {
		QStringList requiredPlugins = d->ProjectConfig.getStringList("Project.RequiredPlugins");
		if (requiredPlugins.contains(pluginID)) {
			requiredPlugins.removeAll(pluginID);
			d->ProjectConfig.setStringList("Project.RequiredPlugins", requiredPlugins);
		}
	}
	
	/*!
		\since YSS 0.17.0
		设置项目所需的插件ID列表到项目配置中。这个函数会覆盖原有的列表。
	*/
	void YSSProject::setRequiredPlugins(const QStringList& pluginIDs) {
		d->ProjectConfig.setStringList("Project.RequiredPlugins", pluginIDs);
	}

	/*!
		\since YSS 0.17.0
		return 当前项目所需的插件ID列表。
	*/
	QStringList YSSProject::getRequiredPlugins() {
		return d->ProjectConfig.getStringList("Project.RequiredPlugins");
	}

	/*!
		\since YSS 0.17.0
		升级项目。这个函数会将项目配置文件升级到最新版本，并在必要时进行数据迁移。
		它会根据当前项目的版本号，逐步应用升级步骤，直到达到最新版本。

		return 是否成功升级。

		请注意，在部分极端情况下，升级可能中途失败，此时项目可能处于不完整状态。建议在升级前备份项目文件。
		
	*/
	bool YSSProject::updateProject() {
		bool rtn = true;
		if (getProjectVersion() == Visindigo::General::Version("0.1")) {
			d->ProjectConfig.setString("Project.Version", "0.2");
			// nothing special to do for 0.2
		}
		return rtn;
	}

	/*!
		\since YSS 0.17.0
		为指定文件\a uri 记录偏好的文件服务ID \a serverID。
	*/
	void YSSProject::setFilePreferredFileServer(const YSSURI& uri, const QString& serverID) {
		if (not uri.isValid()) {
			yErrorF << "Invalid URI, cannot set preferred file server";
			return;
		}
		const QString storedPath = d->uriToStr(uri);
		auto count = d->ProjectConfig.keys("FilePreferredFileServers.filePath").size();
		if (hasFilePreferredFileServer(uri)) {
			for (int i = 0; i < count; ++i) {
				const YSSURI path = d->strToUri(d->ProjectConfig.getString("FilePreferredFileServers.filePath." + QString::number(i)));
				if (path == uri) {
					d->ProjectConfig.setString("FilePreferredFileServers.serverID." + QString::number(i), serverID);
					break;
				}
			}
		}
		else {
		
			d->ProjectConfig.setString("FilePreferredFileServers.filePath." + QString::number(count), storedPath);
			d->ProjectConfig.setString("FilePreferredFileServers.serverID." + QString::number(count), serverID);
		}
	}

	/*!
		\since YSS 0.17.0
		\a uri 指定的文件。
		return 指定文件的偏好文件服务ID，如果没有设置，则返回空字符串。
	*/
	QString YSSProject::getFilePreferredFileServer(const YSSURI& uri) {
		if (not uri.isValid()) {
			return QString();
		}
		auto count = d->ProjectConfig.keys("FilePreferredFileServers.filePath").size();
		for (int i = 0; i < count; ++i) {
			const YSSURI path = d->strToUri(d->ProjectConfig.getString("FilePreferredFileServers.filePath." + QString::number(i)));
			if (path == uri) {
				return d->ProjectConfig.getString("FilePreferredFileServers.serverID." + QString::number(i));
			}
		}
		return QString();
	}

	/*!
		\since YSS 0.17.0
		\a uri 指定的文件。

		删除指定文件的偏好文件服务设置。如果指定文件没有偏好文件服务，则不会有任何操作。

		值得指出的是，使用removeEditorOpenedFile活removeEditorOpenedFilesInArea函数清除
		已打开文件记录时，将会自动调用此函数移除偏好记录。
	*/
	void YSSProject::removeFilePreferredFileServer(const YSSURI& uri) {
		if (not uri.isValid()) {
			return;
		}
		auto count = d->ProjectConfig.keys("FilePreferredFileServers.filePath").size();
		for (int i = 0; i < count; ++i) {
			const YSSURI path = d->strToUri(d->ProjectConfig.getString("FilePreferredFileServers.filePath." + QString::number(i)));
			if (path == uri) {
				d->ProjectConfig.remove("FilePreferredFileServers.filePath." + QString::number(i));
				d->ProjectConfig.remove("FilePreferredFileServers.serverID." + QString::number(i));
				break;
			}
		}
	}

	/*!
		\since YSS 0.17.0
		\a uri 指定的文件。
		return 如果指定文件有偏好文件服务，则返回true，否则返回false。
	*/
	bool YSSProject::hasFilePreferredFileServer(const YSSURI& uri) {
		if (not uri.isValid()) {
			return false;
		}
		auto count = d->ProjectConfig.keys("FilePreferredFileServers.filePath").size();
		for (int i = 0; i < count; ++i) {
			const YSSURI path = d->strToUri(d->ProjectConfig.getString("FilePreferredFileServers.filePath." + QString::number(i)));
			if (path == uri) {
				return true;
			}
		}
		return false;
	}

}