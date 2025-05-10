#include "../YSSProject.h"
#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include "../../Utility/YamlConfig.h"

namespace YSSCore::General {
	class YSSProjectPrivate {
		friend class YSSProject;
	protected:
		YSSCore::Utility::YamlConfig* ProjectConfig = nullptr;
		QString ConfigPath;
	protected:
		YSSProjectPrivate() {
			ProjectConfig = new YSSCore::Utility::YamlConfig();
		}
		~YSSProjectPrivate() {
			if (ProjectConfig != nullptr) {
				delete ProjectConfig;
			}
		}
	};
	YSSProject::YSSProject() {
		d = new YSSProjectPrivate();
	}
	bool YSSProject::loadProject(const QString& configPath) {
		QFile config(configPath);
		if (!config.exists()) {
			return false;
		}
		if (!config.open(QIODevice::ReadOnly)) {
			return false;
		}
		QTextStream in(&config);
		in.setEncoding(QStringConverter::Utf8);
		QString yamlStr = in.readAll();
		config.close();
		d->ConfigPath = configPath;
		return d->ProjectConfig->parse(yamlStr);
	}
	bool YSSProject::saveProject(const QString& configPath) {
		if (configPath.isEmpty()) {
			if (d->ConfigPath.isEmpty()) {
				return false;
			}
		}
		else {
			d->ConfigPath = configPath;
		}
		QString yamlStr = d->ProjectConfig->toString();
		QFile config(configPath);
		if (!config.open(QIODevice::WriteOnly)) {
			return false;
		}
		QTextStream out(&config);
		out.setEncoding(QStringConverter::Utf8);
		out << yamlStr;
		config.close();
		return true;
	}
	QString YSSProject::getProjectName() {
		return d->ProjectConfig->getString("Project.Name");
	}
	QString YSSProject::getProjectDescription() {
		return d->ProjectConfig->getString("Project.Description");
	}
	QString YSSProject::getProjectDebugServerID() {
		return d->ProjectConfig->getString("Project.DebugServerID");
	}
	QString YSSProject::getProjectFolder() {
		return d->ConfigPath;
	}
	QString YSSProject::getProjectIconPath() {
		return d->ProjectConfig->getString("Project.IconPath");
	}
	YSSCore::Utility::YamlConfig* YSSProject::getProjectConfig() {
		return d->ProjectConfig;
	}

} 