#pragma once
#include "../Macro.h"
#include <QtCore/qobject.h>
// Forward declarations
namespace Visindigo::Editor {
	class EditorPlugin;
	class EditorPluginModulePrivate;
}
// Main
namespace Visindigo::Editor {
	class QT_DEPRECATED VisindigoAPI EditorPluginModule :public QObject {
		friend class EditorPluginModulePrivate;
		friend class EditorPlugin;
		Q_OBJECT;
		Q_DISABLE_COPY_MOVE(EditorPluginModule);
	public:
		EditorPluginModule(const QString& moduleName, const QString& moduleID, EditorPlugin* plugin);
		virtual ~EditorPluginModule();
		EditorPlugin* getPlugin() const;
		QString getModuleName() const;
		QString getModuleID() const;
	private:
		EditorPluginModulePrivate* d;
	};
}