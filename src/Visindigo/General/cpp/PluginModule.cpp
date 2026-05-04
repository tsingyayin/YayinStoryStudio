#include "../PluginModule.h"
#include "../Plugin.h"
#include "General/Exception.h"
#include "General/Plugin.h"
#include "General/TranslationHost.h"
namespace Visindigo::General {
	class PluginModulePrivate {
		friend class PluginModule;
	protected:
		Plugin* ParentPlugin;
		QString ModuleName;
		QString ModuleID;
		QString ModuleTypeID;
	};

	/*!
		\class Visindigo::General::PluginModule
		\inheaderfile General/PluginModule.h
		\inmodule Visindigo
		\ingroup VIPlugin
		\brief 此类为Yayin Story Studio 提供插件模块基类。
		\since Visindigo 0.13.0
		PluginModule是插件模块的基类，所有插件模块都需要继承此类。

		插件模块是插件的组成部分，提供了插件的具体功能实现。每个插件模块都属于一个插件。

		moduleID是插件模块在当前插件内的唯一标识符，用于区分同一插件内的不同模块。
		typeID是插件模块的类型标识符，用于区分不同类型的模块。

		moduleID在插件内必须唯一，但不同插件之间则可重复，因此moduleID应该尽量短，
		并建议只用小写下划线命名法，以提高可读性和易用性。而typeID则需要在整个应用程序
		内保持唯一，因此建议使用带有域名限定的命名方式，如"cn.yxgeneral.visindigo.placeholder_provider"。

		插件模块可以在被启用或禁用时执行特定的操作，以实现动态加载和卸载功能。
	*/

	/*!
		\since Visindigo 0.13.0
		\a plugin 所属插件对象指针
		\a moduleID 模块唯一标识符
		\a typeID 模块类型标识符
		\a moduleName 模块名称

		构造函数，创建一个插件模块对象。
	*/
	PluginModule::PluginModule(Plugin* parent, const QString& moduleID, const QString& typeID, const QString& moduleName) :QObject(parent){
		d = new PluginModulePrivate();
		if (not parent) {
			VI_Throw_ST(Exception::NullPointer, "plugin pointer is null");
		}
		d->ParentPlugin = parent;
		d->ParentPlugin->registerPluginModule(this);
		if (moduleName == QString()) {
			d->ModuleName = moduleID;
		}
		else {
			d->ModuleName = moduleName;
		}
		d->ModuleID = moduleID;
		d->ModuleTypeID = typeID;
	}

	/*!
		\since Visindigo 0.13.0
		析构函数。
	*/
	PluginModule::~PluginModule() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		返回所属插件对象指针。
	*/
	Plugin* PluginModule::getPlugin() const {
		return d->ParentPlugin;
	}

	/*!
		\since Visindigo 0.13.0
		返回模块名称。
	*/
	QString PluginModule::getModuleName() const {
		return d->ModuleName;
	}

	/*!
		\since Visindigo 0.15.0
		尝试实用VI18N宏处理模块名称的国际化版本，如果处理失败，则返回原始模块名称。
	*/
	QString PluginModule::getModuleNameI18N() const {
		return VI18N(d->ModuleName);
	}
	/*!
		\since Visindigo 0.13.0
		返回模块唯一标识符。
	*/
	QString PluginModule::getModuleID() const {
		return d->ModuleID;
	}

	/*!
		\since Visindigo 0.13.0
		返回模块类型标识符。
	*/
	QString PluginModule::getModuleTypeID() const {
		return d->ModuleTypeID;
	}

	/*!
		\since Visindigo 0.13.0
		模块被启用时调用的函数。

		默认实现为空函数，子类可重载此函数以实现自定义行为。
	*/
	void PluginModule::onModuleEnable() {
		// Default: Do nothing
	}

	/*!
		\since Visindigo 0.13.0
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