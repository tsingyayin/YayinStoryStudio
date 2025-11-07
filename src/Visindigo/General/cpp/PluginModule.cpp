#include "../PluginModule.h"
#include "../Plugin.h"
namespace Visindigo::General {
	class PluginModulePrivate {
		friend class PluginModule;
	protected:
		Plugin* ParentPlugin;
		QString ModuleName;
		QString ModuleID;
	};

	/*!
		\class Visindigo::General::PluginModule
		\inmodule Visindigo
		\brief 此类为Yayin Story Studio 提供插件模块基类。
		\since Visindigo 0.13.0
		PluginModule是插件模块的基类，所有插件模块都需要继承此类。

	*/
	PluginModule::PluginModule(const QString& moduleName, const QString& moduleID, Plugin* plugin) :QObject(plugin) {
		d = new PluginModulePrivate();
		d->ParentPlugin = plugin;
		d->ModuleName = moduleName;
		d->ModuleID = moduleID;
	}

	PluginModule::~PluginModule() {
		delete d;
	}

	void PluginModule::setPlugin(Plugin* plugin) {
		d->ParentPlugin = plugin;
	}

	Plugin* PluginModule::getPlugin() const {
		return d->ParentPlugin;
	}

	QString PluginModule::getModuleName() const {
		return d->ModuleName;
	}

	QString PluginModule::getModuleID() const {
		return d->ModuleID;
	}

	void PluginModule::onModuleEnable() {
		// Default: Do nothing
	}

	void PluginModule::onModuleDisable() {
		// Default: Do nothing
	}
}