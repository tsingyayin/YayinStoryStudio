#ifndef Visindigo_General_PluginModule_H
#define Visindigo_General_PluginModule_H
#include "../Macro.h"
#include <QtCore/qobject.h>
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
		PluginModule(const QString& moduleName, const QString& moduleID, Plugin* plugin = nullptr);
		virtual ~PluginModule();
		void setPlugin(Plugin* plugin);
		Plugin* getPlugin() const;
		QString getModuleName() const;
		QString getModuleID() const;
	public:
		virtual void onModuleEnable();
		virtual void onModuleDisable();
	private:
		PluginModulePrivate* d;
	};
}

#endif // Visindigo_General_PluginModule_H