#ifndef Visindigo_General_PluginManager_H
#define Visindigo_General_PluginManager_H
#include <QObject>
#include "../VICompileMacro.h"
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
			Success,
			Deactivated,
			PlatformNotSupported,
			InvalidPluginBinary,
			EntryPointNotFound,
			ConstructorError,
			IncompatibleABI,
			IncompatibleAPI,
			AlreadyLoaded,
			DependencyNotFound,
			DependencyLoadFailed,
			MetadataNotFound,
			MetadataNotSame,
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
		void setPluginDeactivate(const QString& id, bool deactivate);
		bool isPluginDeactivate(const QString& id) const;
		void loadDeactivatePluginList();
		QStringList getDeactivatedPluginIDList() const;
		qint32 getLoadedPluginCount() const;
		qint32 getEnabledPluginCount() const;
		Plugin* getPluginByID(const QString& id) const;
		Plugin* getPluginByName(const QString& name) const;
		LoadPluginResult getPluginLoadResultByID(const QString& id) const;
		QList<Plugin*> getLoadedPlugins() const;
		QMap<QString, LoadPluginResult> getAllPluginLoadResults() const;
		void setPluginTypeDescription(const QString& typeID, const QString& typeName, const QString& description);
		QString getPluginTypeName(const QString& typeID) const;
		QString getPluginTypeDescription(const QString& typeID) const;
		QStringList getAllPluginTypeID() const;
		void setPluginModuleTypeDescription(const QString& typeID, const QString& typeName, const QString& description);
		QString getPluginModuleTypeName(const QString& typeID) const;
		QString getPluginModuleTypeDescription(const QString& typeID) const;
		QStringList getAllPluginModuleTypeID() const;
	private:
		PluginManagerPrivate* d;
	};
}
#endif // Visindigo_General_PluginManager_H

#define VIPLM Visindigo::General::PluginManager::getInstance()