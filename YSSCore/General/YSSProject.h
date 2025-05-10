#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>

namespace YSSCore::Utility {
	class YamlConfig;
}
namespace YSSCore::General {
	class YSSProjectPrivate;
	class YSSCoreAPI YSSProject {
		friend class YSSProjectPrivate;
	public:
		YSSProject();
		bool loadProject(const QString& configPath);
		bool saveProject(const QString& configPath = "");
		QString getProjectName();
		QString getProjectDescription();
		QString getProjectDebugServerID();
		QString getProjectFolder();
		QString getProjectIconPath();
		YSSCore::Utility::YamlConfig* getProjectConfig();
	private:
		YSSProjectPrivate* d;
	};
}