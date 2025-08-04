#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"

class QString;
namespace Visindigo::Editor {
	class EditorPluginManagerPrivate;
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