#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>
// Forward declarations
class QDateTime;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::General {
	class Version;
	class YSSProjectPrivate;
}
// Main
namespace YSSCore::General {
	class YSSCoreAPI YSSProject {
		friend class YSSProjectPrivate;
	public:
		YSSProject();
		virtual ~YSSProject();
		bool loadProject(const QString& configPath);
		bool saveProject(const QString& configPath = "");
		bool initProject(const QString& folder, const QString& name);
		QString getProjectName();
		QString getProjectDescription();
		QString getProjectFolder();
		QString getProjectPath();
		QString getProjectIconPath();
		QDateTime getProjectCreateTime();
		QDateTime getProjectLastModifyTime();
		Visindigo::General::Version getProjectVersion();
		Visindigo::Utility::JsonConfig* getProjectConfig();
		QStringList getEditorOpenedFiles(); // return all in absolute path
		void addEditorOpenedFile(const QString& abs_filePath);
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