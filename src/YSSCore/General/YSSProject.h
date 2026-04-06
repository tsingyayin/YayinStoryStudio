#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>
// Forward declarations
class QDateTime;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::General {
	class Plugin;
	class Version;
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
		bool saveProject(const QString& configPath = "");
		bool initProject(const QString& folder, const QString& name);
		QString getProjectName();
		QString getProjectDescription();
		QString getProjectFolder();
		QString getProjectPath();
		QString getProjectIconPath();
		void setProjectName(const QString& name);
		void setProjectDescription(const QString& description);
		void setProjectIconPath(const QString& iconPath);
		QDateTime getProjectCreateTime();
		QDateTime getProjectLastModifyTime();
		QString getProjectDebugServerID();
		void setProjectDebugServerID(const QString& id);
		Visindigo::General::Version getProjectVersion();
		Visindigo::Utility::JsonConfig getProjectConfigForPlugin(Visindigo::General::Plugin* plugin);
		Visindigo::Utility::JsonConfig getProjectConfigForPlugin(const QString& pluginID);
		void saveProjectConfigForPlugin(Visindigo::General::Plugin* plugin, const Visindigo::Utility::JsonConfig& config);
		void saveProjectConfigForPlugin(const QString& pluginID, const Visindigo::Utility::JsonConfig& config);
		QStringList getEditorOpenedFiles(); // return all in absolute path
		void addEditorOpenedFile(const QString& abs_filePath);
		void setEditorOpenedFiles(const QStringList& abs_filePaths);
		void removeEditorOpenedFile(const QString& abs_filePath);
		void removeAllEditorOpenedFiles();
		void setFocusedFile(const QString& abs_filePath);
		QString getFocusedFile(); // return absolute path
		QString getFocusedFileName(); // return only file name
		void refreshLastModifyTime();
		static YSSProject* getCurrentProject();
		static void setCurrentProject(YSSProject* project);
	private:
		YSSProjectPrivate* d;
	};
}