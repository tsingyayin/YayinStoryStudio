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
		\inheaderfile General/PluginModule.h
		\inmodule Visindigo
		\ingroup VPlugin
		\brief 此类为Yayin Story Studio 提供插件模块基类。
		\since Visindigo 0.13.0
		PluginModule是插件模块的基类，所有插件模块都需要继承此类。

	*/

	/*!
		\a moduleName 模块名称
		\a moduleID 模块唯一标识符
		\a plugin 所属插件对象指针

		构造函数，创建一个插件模块对象。
	*/
	PluginModule::PluginModule(const QString& moduleName, const QString& moduleID, Plugin* plugin) :QObject(plugin) {
		d = new PluginModulePrivate();
		d->ParentPlugin = plugin;
		d->ModuleName = moduleName;
		d->ModuleID = moduleID;
	}

	/*!
		析构函数。
	*/
	PluginModule::~PluginModule() {
		delete d;
	}

	/*!
		\a plugin 所属插件对象指针。

		设置所属插件对象。
	*/
	void PluginModule::setPlugin(Plugin* plugin) {
		d->ParentPlugin = plugin;
	}

	/*!
		返回所属插件对象指针。
	*/
	Plugin* PluginModule::getPlugin() const {
		return d->ParentPlugin;
	}

	/*!
		返回模块名称。
	*/
	QString PluginModule::getModuleName() const {
		return d->ModuleName;
	}

	/*!
		返回模块唯一标识符。
	*/
	QString PluginModule::getModuleID() const {
		return d->ModuleID;
	}

	/*!
		模块被启用时调用的函数。

		默认实现为空函数，子类可重载此函数以实现自定义行为。
	*/
	void PluginModule::onModuleEnable() {
		// Default: Do nothing
	}

	/*!
		模块被禁用时调用的函数。

		默认实现为空函数，子类可重载此函数以实现自定义行为。
	*/
	void PluginModule::onModuleDisable() {
		// Default: Do nothing
	}
}

/*!
	\macro VISPM 
	\since Visindigo 0.13.0
	\relates Visindigo::General::PluginManager

	等同于 Visindigo::General::PluginManager::getInstance()，用于快速访问插件管理器单例对象。
*/