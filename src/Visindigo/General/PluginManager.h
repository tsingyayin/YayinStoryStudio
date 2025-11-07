#ifndef Visindigo_General_PluginManager_H
#define Visindigo_General_PluginManager_H
#include <QtCore/qobject.h>
#include "../Macro.h"
// Forward declarations
class QString;
namespace Visindigo::General {
	class Plugin;
	class PluginManagerPrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI PluginManager :public QObject{
		friend class PluginManagerPrivate;
		Q_OBJECT;
	private:
		PluginManager(QObject* parent = nullptr);
	public:
		static PluginManager* getInstance();
		~PluginManager();
		void loadAllPlugin();
		void enableAllPlugin();
		bool isPluginEnable(const QString& id) const;
		Plugin* getPluginByID(const QString& id) const;
		Plugin* getPluginByName(const QString& name) const;
	private:
		PluginManagerPrivate* d;
	};
}
#endif // Visindigo_General_PluginManager_H