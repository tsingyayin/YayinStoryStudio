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

	EditorPluginModule::EditorPluginModule(const QString& moduleName, const QString& moduleID, EditorPlugin* plugin) :QObject(plugin),
		d(new EditorPluginModulePrivate()) {
		d->ModuleName = moduleName;
		d->ModuleID = moduleID;
		d->Plugin = plugin;
	}

	EditorPluginModule::~EditorPluginModule() {
		delete d;
	}

	EditorPlugin* EditorPluginModule::getPlugin() const {
		return d->Plugin;
	}

	QString EditorPluginModule::getModuleName() const {
		return d->ModuleName;
	}

	QString EditorPluginModule::getModuleID() const {
		return d->ModuleID;
	}
}