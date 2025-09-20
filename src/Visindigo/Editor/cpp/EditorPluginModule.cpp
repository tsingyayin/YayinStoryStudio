#include "../EditorPluginModule.h"
#include "../EditorPlugin.h"

namespace Visindigo::Editor {
	class EditorPluginModulePrivate {
		friend class EditorPluginModule;
	protected:
		EditorPlugin* Plugin = nullptr;
		QString ModuleName;
		QString ModuleID;
	};

	/*!
		\class Visindigo::Editor::EditorPluginModule
		\inmodule Visindigo
		\brief 此类为Yayin Story Studio 提供插件模块基类。
		\since Visindigo 0.13.0

		EditorPluginModule是所有插件模块的基类，所有可以被视为一个插件的子功能的类（模块）都可以继承此类。

		由Visindigo提供的EditorPluginModule子类当前包括LangServer、DebugServer、FileServer等。

		此类的使用不是强制性的——用户可以以任何自由方式编写插件，并自行决定插件的架构和模块划分。
		但总的来说仍然建议使用此类，以便于插件的管理和维护。
	*/
	EditorPluginModule::EditorPluginModule(const QString& moduleName, const QString& moduleID, EditorPlugin* plugin) :QObject(plugin),
		d(new EditorPluginModulePrivate()) {
		d->ModuleName = moduleName;
		d->ModuleID = moduleID;
		d->Plugin = plugin;
	}

	/*!
		\since Visindigo 0.13.0
		析构EditorPluginModule对象。一般来说，没有任何情况需要手动析构此对象。EditorPluginModule应该与它所属的插件有一致的生命周期。
	*/
	EditorPluginModule::~EditorPluginModule() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		返回此模块所属的插件。
	*/
	EditorPlugin* EditorPluginModule::getPlugin() const {
		return d->Plugin;
	}

	/*!
		\since Visindigo 0.13.0
		返回此模块的名称。
	*/
	QString EditorPluginModule::getModuleName() const {
		return d->ModuleName;
	}

	/*!
		\since Visindigo 0.13.0
		返回此模块的ID。
	*/
	QString EditorPluginModule::getModuleID() const {
		return d->ModuleID;
	}
}