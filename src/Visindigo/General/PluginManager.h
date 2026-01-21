#ifndef Visindigo_General_PluginManager_H
#define Visindigo_General_PluginManager_H
#include <QObject>
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
	signals:
		void pluginLoaded(Plugin* plugin);
		void pluginEnabled(Plugin* plugin);
	private:
		PluginManager(QObject* parent = nullptr);
	public:
		static PluginManager* getInstance();
		~PluginManager();
		void loadAllPlugin();
		void enableAllPlugin();
		void testAllPlugin();
		void applicationInitAllPlugin();
		void disableAllPlugin();
		bool isPluginEnable(const QString& id) const;
		bool isPluginEnable(Plugin* plugin) const;
		qint32 getLoadedPluginCount() const;
		qint32 getEnabledPluginCount() const;
		Plugin* getPluginByID(const QString& id) const;
		Plugin* getPluginByName(const QString& name) const;
	private:
		PluginManagerPrivate* d;
	};
}
#endif // Visindigo_General_PluginManager_H

#define VISPM Visindigo::General::PluginManager::getInstance()