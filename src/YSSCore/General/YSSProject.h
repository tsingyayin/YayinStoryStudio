#ifndef YSSCore_General_YSSProject_h
#define YSSCore_General_YSSProject_h
#include "YSSCoreCompileMacro.h"
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
	class YSSCoreAPI YSSProject {
		friend class YSSProjectPrivate;
	public:
		enum LoadProjectResult {
			Unknown = 0,
			Success,
			ParseError,
			InvalidConfig
		};
	public:
		YSSProject();
		virtual ~YSSProject();
		LoadProjectResult loadProject(const QString& configPath);
		QString getProjectConfigPath();
		bool saveProject(const QString& configPath = "");
		bool initProject(const QString& folder, const QString& name);
		QString getProjectName();
		QString getProjectDescription();
		QString getProjectFolder();
		QString getProjectPath();
		QString getProjectIconPath();
		QString getProjectAuthor();
		void setProjectName(const QString& name);
		void setProjectDescription(const QString& description);
		void setProjectIconPath(const QString& iconPath);
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
		QStringList getEditorOpenedFiles(); // return all in absolute path
		QMap<QString, QStringList> getEditorOpenedFilesInArea(); // return all in absolute path
		void addEditorOpenedFile(const QString& abs_filePath, const QString& areaID = "");
		void moveEditorOpenedFile(const QString& abs_filePath, const QString& fromID, const QString& toID);
		void setEditorOpenedFiles(const QStringList& abs_filePaths);
		void setEditorOpenedFilesInArea(const QMap<QString, QStringList>& abs_filePathsInArea);
		void removeEditorOpenedFile(const QString& abs_filePath);
		void removeAllEditorOpenedFiles();
		void removeEditorOpenedFilesInArea(const QString& areaID);
		void setFocusedFile(const QString& abs_filePath);
		QString getFileAreaID(const QString& abs_filePath);
		QString getFocusedFile(); // return absolute path
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
		static YSSProject* getCurrentProject();
		static void setCurrentProject(YSSProject* project);
	private:
		YSSProjectPrivate* d;
	};
}
#endif // YSSCore_General_YSSProject_h
