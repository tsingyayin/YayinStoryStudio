#ifndef Visindigo_General_PluginManager_h
#define Visindigo_General_PluginManager_h
#include <QObject>
#include "VICompileMacro.h"
// Forward declarations
class QString;
class QDir;
namespace Visindigo::General {
	class Plugin;
	class PluginManagerPrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI PluginManager :public QObject {
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
			InvalidDependLib,
			DependLibConflict,
			EntryPointNotFound,
			ConstructorError,
			IncompatibleABI,
			IncompatibleAPI,
			AlreadyLoaded,
			DependencyNotFound,
			DependencyLoadFailed,
			DependencyDeactivated,
			MetadataNotFound,
			MetadataNotSame,
		};
		Q_ENUM(LoadPluginResult);
		enum class PluginState {
			Unknown = 0,
			Deactivated,
			MetadataLoaded,
			InstanceCreated,
			Enabled,
			Disabled,
		};
		Q_ENUM(PluginState);
	private:
		PluginManager(QObject* parent = nullptr);
	public:
		static PluginManager* getInstance();
		~PluginManager();
		void addPluginLoadPath(const QString& libFilePath, const QString& metaJsonFilePath, const QString& pluginBinaryFolderPath = "");
		void addPluginEntryPoint(Plugin* (*entryPoint)(void), const QString& metaJsonFilePath, const QString& pluginBinaryFolderPath = "");
		void loadAllPlugin();
		void enableAllPlugin();
		void testAllPlugin();
		void applicationInitAllPlugin();
		void disableAllPlugin();
		void unloadAllPlugin();
		bool isPluginEnable(const QString& id) const;
		bool isPluginEnable(Plugin* plugin) const;
		void setPluginDeactivate(const QString& id, bool deactivate);
		bool isPluginDeactivate(const QString& id) const;
		void setDeactivatePluginList(const QStringList& deactivatedList);
		QStringList getDeactivatedPluginIDList() const;
		qint32 getLoadedPluginCount() const;
		qint32 getEnabledPluginCount() const;
		Plugin* getPluginByID(const QString& id) const;
		QList<Plugin*> getPluginByName(const QString& name) const;
		QDir getPluginBinaryFolder(const QString& id) const;
		LoadPluginResult getPluginLoadResultByID(const QString& id) const;
		QList<Plugin*> getLoadedPlugins() const;
		QList<Plugin*> getEnabledPlugins() const;
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
#endif // Visindigo_General_PluginManager_h

#define VIPLM Visindigo::General::PluginManager::getInstance()