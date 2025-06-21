#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>

class QDateTime;
namespace YSSCore::Utility {
	class JsonConfig;
}
namespace YSSCore::General {
	class Version;
	class YSSProjectPrivate;
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
		QString getProjectDebugServerID();
		QString getProjectFolder();
		QString getProjectPath();
		QString getProjectIconPath();
		QDateTime getProjectCreateTime();
		QDateTime getProjectLastModifyTime();
		Version getProjectVersion();
		YSSCore::Utility::JsonConfig* getProjectConfig();
		QStringList getEditorOpenedFiles(); // return all in absolute path
		void addEditorOpenedFile(const QString& abs_filePath);
		void removeEditorOpenedFile(const QString& abs_filePath);
		void removeAllEditorOpenedFiles();
		void setFocusedFile(const QString& abs_filePath);
		QString getFocusedFile(); // return absolute path
		void refreshLastModifyTime();
		
	private:
		YSSProjectPrivate* d;
	};
}