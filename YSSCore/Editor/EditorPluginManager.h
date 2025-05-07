#pragma once
#include <QObject>
#include <QString>
#include "../Macro.h"

namespace YSSCore::Editor {
	class EditorPluginManagerPrivate;
	class YSSCoreAPI EditorPluginManager : public QObject {
		friend class EditorPluginManagerPrivate;
		Q_OBJECT
	public:
		EditorPluginManager(QObject* parent = nullptr);
		~EditorPluginManager();
		void programLoadPlugin();
		void loadPlugin();
		bool isPluginEnable(const QString& id) const;
	private:
		EditorPluginManagerPrivate* p;
	};
}