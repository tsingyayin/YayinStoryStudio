#include "../YSSProject.h"
#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include "../Version.h"
#include "../Log.h"
#include "../../Utility/JsonConfig.h"

namespace YSSCore::General {
	class YSSProjectPrivate {
		friend class YSSProject;
	protected:
		YSSCore::Utility::JsonConfig* ProjectConfig = nullptr;
		QString ConfigPath;
	protected:
		YSSProjectPrivate() {
			ProjectConfig = new YSSCore::Utility::JsonConfig();
		}
		~YSSProjectPrivate() {
			if (ProjectConfig != nullptr) {
				delete ProjectConfig;
			}
		}
		void updateLastModifyTime() {
			ProjectConfig->setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
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
		return d->ProjectConfig->parse(yamlStr).error == QJsonParseError::NoError;
	}
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
		QString yamlStr = d->ProjectConfig->toString();
		yDebug << d->ConfigPath;
		yDebug << yamlStr;
		QFile config(d->ConfigPath);
		if (!config.open(QIODevice::WriteOnly)) {
			yDebug << "false";
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
		QFileInfo info(d->ConfigPath);
		return info.absolutePath();
	}
	QString YSSProject::getProjectPath() {
		return d->ConfigPath;
	}
	QString YSSProject::getProjectIconPath() {
		return d->ProjectConfig->getString("Project.IconPath");
	}
	QDateTime YSSProject::getProjectCreateTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.CreateTime"), "yyyy-MM-dd hh:mm:ss");
	}
	QDateTime YSSProject::getProjectLastModifyTime() {
		return QDateTime::fromString(d->ProjectConfig->getString("Project.LastModifyTime"), "yyyy-MM-dd hh:mm:ss");
	}
	Version YSSProject::getProjectVersion() {
		return Version(d->ProjectConfig->getString("Project.Version"));
	}
	YSSCore::Utility::JsonConfig* YSSProject::getProjectConfig() {
		return d->ProjectConfig;
	}
	void YSSProject::refreshLastModifyTime() {
		d->ProjectConfig->setString("Project.LastModifyTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	}
	bool YSSProject::createProject(const QString& folder, const QString& name) {
		QDir dir(folder);
		if (dir.exists()) {
			return false;
		}
		dir.mkpath("./");
		QString configPath = folder + "/yssproj.yml";
		QFile config(configPath);
		if (!config.open(QIODevice::WriteOnly)) {
			return false;
		}
		QString yamlStr = "Project:\n  Name: " + name + "\n  Description: \"\"\n  DebugServerID: \"\"\n  IconPath: \"\"\n  CreateTime: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n  LastModifyTime: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n  Version: " + Compiled_YSSABI_Version.toString();
		QTextStream out(&config);
		out.setEncoding(QStringConverter::Utf8);
		out << yamlStr;
		config.close();
		return true;
	}
} 