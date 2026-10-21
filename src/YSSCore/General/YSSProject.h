#ifndef YSSCore_General_YSSProject_h
#define YSSCore_General_YSSProject_h
#include "YSSCoreCompileMacro.h"
#include "General/YSSURI.h"
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
// Forward declarations
class QDateTime;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::General {
	class Plugin;
	class Version;
}
namespace YSSCore::General {
	class YSSProjectPrivate;
}
// Main
namespace YSSCore::General {
	class YSSCoreAPI YSSProject : public QObject {
		Q_OBJECT;
		friend class YSSProjectPrivate;
	public:
		enum class LoadError {
			None = 0,
			Unknown,
			InvalidUri,
			NotFound,
			ReadFailed,
			ParseError,
			InvalidConfig
		};
		Q_ENUM(LoadError)
	signals:
		void projectLoaded();
		void projectLoadFailed(YSSCore::General::YSSProject::LoadError error);
		void projectSaved();
		void projectSaveFailed();
	public:
		static YSSProject* getCurrentProject();
		static void setCurrentProject(YSSProject* project);
		static Visindigo::General::Version getCurrentProjectVersion();
		static QMap<Visindigo::General::Version, std::tuple<Visindigo::General::Version, Visindigo::General::Version>> getProjectVersionRange();
	public:
		explicit YSSProject(QObject* parent = nullptr);
		virtual ~YSSProject();
	public:
		void loadProject(const YSSURI& configPath);
		void saveProject(const YSSURI& configPath = YSSURI());
		bool initProject(const YSSURI& folder, const QString& name);
		bool isLoaded() const;
		bool isSaving() const;
		LoadError getLoadError() const;
		QString getProjectName();
		QString getProjectDescription();
		YSSURI getProjectFolder();
		YSSURI getProjectCacheFolder();
		YSSURI getProjectPath();
		YSSURI getProjectIconPath();
		QString getProjectAuthor();
		void setProjectName(const QString& name);
		void setProjectDescription(const QString& description);
		void setProjectIconPath(const YSSURI& iconPath);
		void setProjectAuthor(const QString& author);
		QDateTime getProjectCreateTime();
		QDateTime getProjectLastModifyTime();
		QString getProjectDebugServerID();
		void setProjectDebugServerID(const QString& id);
		Visindigo::General::Version getProjectVersion();
		Visindigo::Utility::JsonConfig getProjectConfigForPlugin(Visindigo::General::Plugin* plugin);
		Visindigo::Utility::JsonConfig getProjectConfigForPlugin(const QString& pluginID);
		Visindigo::Utility::JsonConfig getTreeLayoutConfig();
		bool hasTreeLayoutConfig();
		void setTreeLayoutConfig(const Visindigo::Utility::JsonConfig& config);
		void saveProjectConfigForPlugin(Visindigo::General::Plugin* plugin, const Visindigo::Utility::JsonConfig& config);
		void saveProjectConfigForPlugin(const QString& pluginID, const Visindigo::Utility::JsonConfig& config);
		QList<YSSURI> getEditorOpenedFiles(); // return all as URIs
		QMap<QString, QList<YSSURI>> getEditorOpenedFilesInArea(); // return all as URIs
		void addEditorOpenedFile(const YSSURI& uri, const QString& areaID = "");
		void moveEditorOpenedFile(const YSSURI& uri, const QString& fromID, const QString& toID);
		void setEditorOpenedFiles(const QList<YSSURI>& uris);
		void setEditorOpenedFilesInArea(const QMap<QString, QList<YSSURI>>& urisInArea);
		void removeEditorOpenedFile(const YSSURI& uri);
		void removeAllEditorOpenedFiles();
		void removeEditorOpenedFilesInArea(const QString& areaID);
		void setFocusedFile(const YSSURI& uri);
		QString getFileAreaID(const YSSURI& uri);
		YSSURI getFocusedFile();
		QString getFocusedFileName(); // return only file name
		void refreshLastModifyTime();
		void createFileBackup(const QString& inProjRelativePth);
		QList<QDateTime> getFileBackupList(const QString& inProjRelativePth);
		QByteArray getFileBackupContent(const QString& inProjRelativePth, const QDateTime& backupTime);
		bool restoreFileBackup(const QString& inProjRelativePth, const QDateTime& backupTime, const QString& restoreToPath = "", bool overwrite = false);
		void removeFileBackup(const QString& inProjRelativePth, const QDateTime& backupTime);
		void removeFileBackup(const QString& inProjRelativePth);
		void removeAllFileBackup();
		QStringList getAllFileBackups();
		void setFileBackupMaxCount(qint32 count);
		qint32 getFileBackupMaxCount();
		void addRequiredPlugin(const QString& pluginID);
		void removeRequiredPlugin(const QString& pluginID);
		void setRequiredPlugins(const QStringList& pluginIDs);
		QStringList getRequiredPlugins();
		bool updateProject();
		void setFilePreferredFileServer(const YSSURI& uri, const QString& serverID);
		QString getFilePreferredFileServer(const YSSURI& uri);
		void removeFilePreferredFileServer(const YSSURI& uri);
		bool hasFilePreferredFileServer(const YSSURI& uri);
	private:
		YSSProjectPrivate* d;
	};
}

Q_DECLARE_METATYPE(YSSCore::General::YSSProject::LoadError)
#endif // YSSCore_General_YSSProject_h
