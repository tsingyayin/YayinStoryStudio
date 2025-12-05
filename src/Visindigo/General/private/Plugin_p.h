#ifndef Visinidgo_General_Plugin_p_H
#define Visinidgo_General_Plugin_p_H
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qdir.h>
#include "../Version.h"
#include "../../Utility/JsonConfig.h"
namespace Visindigo::General {
	class Plugin;
	class PluginManager;
	class PluginModule;
}
namespace Visindigo::__Private__ {
	class PluginPrivate {
		friend class Visindigo::General::Plugin;
		friend class Visindigo::General::PluginManager;
	protected:
		Visindigo::General::Plugin* q = nullptr;
		Visindigo::General::Version ABIVersion;
		Visindigo::General::Version PluginVersion;	
		bool TestEnable = false;
		QString PluginID;
		QString PluginName;
		QStringList PluginAuthor;
		QDir PluginFolder;
		QString PluginExtensionID;
		Visindigo::Utility::JsonConfig Config;
		QList<Visindigo::General::PluginModule*> Modules;
	protected:
		void initializePluginFolder(const QDir& baseDir);
		bool enablePlugin();
		bool disablePlugin();
	};
}
#endif // Visinidgo_General_Plugin_p_H