#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"
// Forward declarations
class QString;
namespace Visindigo::Editor {
	class EditorPluginManagerPrivate;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI EditorPluginManager : public QObject {
		friend class EditorPluginManagerPrivate;
		Q_OBJECT
	public:
		EditorPluginManager(QObject* parent = nullptr);
		~EditorPluginManager();
		void programLoadPlugin();
		void loadPlugin();
		bool isPluginEnable(const QString& id) const;
	private:
		EditorPluginManagerPrivate* d;
	};
}