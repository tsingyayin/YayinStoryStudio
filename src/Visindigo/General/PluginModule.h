#ifndef Visindigo_General_PluginModule_H
#define Visindigo_General_PluginModule_H
#include "../Macro.h"
#include <QtCore/qobject.h>

#define VIModuleType_PlaceholderProvider "cn.yxgeneral.visindigo.placeholder_provider"
#define VIModuleType_CommandHandler "cn.yxgeneral.visindigo.command_handler"
#define VIModuleType_Translator "cn.yxgeneral.visindigo.translator"
#define VIModuleType_Unknown "__unknown__"
// Forward declarations
namespace Visindigo::General {
	class Plugin;
	class PluginModulePrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI PluginModule :public QObject {
		friend class PluginModulePrivate;
		friend class Plugin;
		Q_OBJECT;
	public:
		PluginModule(Plugin* parent, const QString& moduleID, const QString& typeID = VIModuleType_Unknown, const QString& moduleName = QString());
		virtual ~PluginModule();
		Plugin* getPlugin() const;
		QString getModuleName() const;
		QString getModuleID() const;
		QString getModuleTypeID() const;
	public:
		virtual void onModuleEnable();
		virtual void onModuleDisable();
	private:
		PluginModulePrivate* d;
	};
}

#endif // Visindigo_General_PluginModule_H