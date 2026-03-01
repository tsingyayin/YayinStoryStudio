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
	public:
		enum class LoadPluginResult {
			Unknown = 0,
			Success = 1,
			PlatformNotSupported = 2,
			InvalidPlugin = 3,
			IncompatibleABI = 4,
			IncompatibleAPI = 5,
			AlreadyLoaded = 6,
			DependencyNotFound = 7,
			DependencyLoadFailed = 8,
			MetadataNotFound = 8,
			MetadataNotSame = 9,
			PluginIDConflict = 10,
		};
		Q_ENUM(LoadPluginResult);
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
		LoadPluginResult getPluginLoadResultByID(const QString& id) const;
		LoadPluginResult getPluginLoadResultByName(const QString& name) const;
		QList<Plugin*> getLoadedPlugins() const;
		QMap<QString, LoadPluginResult> getAllPluginLoadResults() const;
	private:
		PluginManagerPrivate* d;
	};
}
#endif // Visindigo_General_PluginManager_H

#define VISPM Visindigo::General::PluginManager::getInstance()