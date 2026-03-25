#ifndef Visinidgo_General_Plugin_p_H
#define Visinidgo_General_Plugin_p_H
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qdir.h>
#include "../Version.h"
#include "../../Utility/JsonConfig.h"
#include "General/Plugin.h"
namespace Visindigo::General {
	class Plugin;
	class PluginManager;
	class PluginModule;
	class VIApplication;
	class Logger;
}
namespace Visindigo::__Private__ {
	class PluginPrivate {
		friend class Visindigo::General::Plugin;
		friend class Visindigo::General::PluginManager;
		friend class Visindigo::General::VIApplication;
	protected:
		Visindigo::General::Plugin* q = nullptr;
		Visindigo::General::Version APIVersion;
		Visindigo::General::Version ABIVersion;
		Visindigo::General::Version PluginVersion;
		Visindigo::General::Plugin::LoadType LoadType = Visindigo::General::Plugin::Unknown;
		bool TestEnable = false;
		QString PluginID;
		QString PluginName;
		QStringList PluginAuthor;
		QDir PluginFolder;
		QString PluginExtensionID;
		Visindigo::Utility::JsonConfig Config;
		QList<Visindigo::General::PluginModule*> Modules;
		Visindigo::General::Logger* Logger = nullptr;
		QMap<QString, Visindigo::General::PluginModule*> ModuleIDMap;
		QMap<QString, QList<Visindigo::General::PluginModule*>> ModuleTypeIDMap;
	protected:
		void initializePluginFolder(const QDir& baseDir);
		void setPluginLoadType(Visindigo::General::Plugin::LoadType loadType);
		void loadConfig();
		bool enablePlugin();
		bool disablePlugin();
	};
}
#endif // Visinidgo_General_Plugin_p_H