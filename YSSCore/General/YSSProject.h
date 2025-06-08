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
		void initProject(const QString& folder, const QString& name);
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
		void refreshLastModifyTime();
		
	private:
		YSSProjectPrivate* d;
	};
}