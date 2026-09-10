#ifndef Visindigo_General_Plugin_h
#define Visindigo_General_Plugin_h
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "VICompileMacro.h"
#include "General/Version.h"
// Forward declarations
class QString;
class QDir;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::__Private__ {
	class PluginPrivate;
}
namespace Visindigo::General {
	class Translator;
	class PluginManager;
	class PluginModule;
	class Logger;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI Plugin :public QObject {
		friend class PluginManager;
		friend class PluginModule;
		friend class Visindigo::__Private__::PluginPrivate;
		friend class VIApplication;
		Q_OBJECT;
	public:
		enum LoadType {
			Unknown = 0,
			FromDisk,
			FromMemory,
			MainPlugin
		};
		enum class Permission : quint64{
			Unknown = 0x00000000,
			FileRead = 0x00000001,
			FileWrite = 0x00000002,
			NetworkLocal = 0x00000004,
			Network = 0x00000008,
			NetworkCellular = 0x00000010,
			Bluetooth = 0x00000020,
			Audio = 0x00000040,
			Location = 0x00000080,
			LaunchExternal = 0x00000100,
			ModifySystemSettings = 0x00000200,
			RunAnyCommand = 0x00000400,
			ScreenCapture = 0x00000800,
			BackgroundService = 0x00001000,
			Camera = 0x00002000,
			Microphone = 0x00004000,
			Clipboard = 0x00008000,
			CrossPluginAccess = 0x00010000,
			ProcessAccess = 0x00020000,
		};
		Q_ENUM(Permission);
		Q_DECLARE_FLAGS(Permissions, Permission)
	public:
		Plugin(const QString& id, Visindigo::General::Version apiVersion = Compiled_VIAPI_Version, Visindigo::General::Version abiVersion = Compiled_VIABI_Version, QString extensionID = "Visindigo_Base", QObject* parent = nullptr);
		virtual ~Plugin();
		void setTestEnable();
		bool isTestEnable() const;
		QString getPluginID() const;
		QString getPluginName() const;
		QString getPluginNameI18N() const;
		QStringList getPluginAuthor() const;
		QString getPluginDescription() const;
		QString getPluginDescriptionI18N() const;
		QDir getPluginFolder() const;
		QDir getPluginBinaryFolder() const;
		Visindigo::Utility::JsonConfig* getPluginConfig();
		void savePluginConfig();
		void reloadPluginConfig();
		bool testPluginConfig(const QString& node) const;
		LoadType getPluginLoadType() const;
		QList<PluginModule*> getModules() const;
		PluginModule* getModuleByID(const QString& id) const;
		QList<PluginModule*> getModuleByTypeID(const QString& typeID) const;
		Visindigo::General::Version getPluginVersion() const;
		Visindigo::General::Version getPluginABIVersion() const;
		Visindigo::General::Version getPluginAPIVersion() const;
		QString getPluginExtensionID() const;
		bool hasPermission(Permission perm);
		Logger* getLogger() const; 
		Permissions getRequiredPermissions();
	public:
		virtual void onPluginEnable() {};
		virtual void onTest() {};
		virtual void onApplicationInit() {};
		virtual void onPluginDisable() {};
		virtual QWidget* getConfigWidget();
	protected:
		void setRequiredPermissions(Permissions perms);
		void setPluginVersion(const Visindigo::General::Version& version);
		void setPluginName(const QString& name);
		void setPluginAuthor(const QStringList& author);
		void setPluginDescription(const QString& description);
		void registerPluginModule(PluginModule* module);
		void unregisterPluginModule(PluginModule* module);
		void registerColorScheme(const QString& schemeFilePath);
		void registerStyleTemplate(const QString& templateFilePath);
	protected:
		Visindigo::__Private__::PluginPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(Plugin::Permissions)
}

#define Visindigo_PluginMain_Function_Name "VisindigoPluginMain"
typedef Visindigo::General::Plugin* (*__VisindigoPluginMain)(void);

#define VIPlugin(PluginClass)\
public:\
	static PluginClass* getInstance();
#define VIPluginInstance(PluginClass) PluginClass::getInstance()

#endif // Visindigo_General_Plugin_h
