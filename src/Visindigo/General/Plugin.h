#ifndef Visindigo_General_Plugin_H
#define Visindigo_General_Plugin_H
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
	public:
		Plugin(Visindigo::General::Version apiVersion = Compiled_VIAPI_Version, Visindigo::General::Version abiVersion = Compiled_VIABI_Version, QString extensionID = "Visindigo_Base", QObject * parent = nullptr);
		virtual ~Plugin();
		void setTestEnable();
		bool isTestEnable() const;
		virtual void onPluginEnable() {};
		virtual void onTest() {};
		virtual void onApplicationInit() {};
		virtual void onPluginDisable() {};
		QString getPluginID() const;
		QString getPluginName() const;
		QStringList getPluginAuthor() const;
		QDir getPluginFolder() const;
		Visindigo::Utility::JsonConfig* getPluginConfig();
		void savePluginConfig();
		void reloadPluginConfig();
		LoadType getPluginLoadType() const;
		QList<PluginModule*> getModules() const;
		PluginModule* getModuleByID(const QString& id) const;
		QList<PluginModule*> getModuleByTypeID(const QString& typeID) const;
		Visindigo::General::Version getPluginVersion() const;
		Visindigo::General::Version getPluginABIVersion() const;
		Visindigo::General::Version getPluginAPIVersion() const;
		QString getPluginExtensionID() const;
		Logger* getLogger() const;
		virtual QWidget* getConfigWidget();
	protected:
		void setPluginVersion(const Visindigo::General::Version& version);
		void setPluginID(const QString& id);
		void setPluginName(const QString& name);
		void setPluginAuthor(const QStringList& author);
		void registerPluginModule(PluginModule* module);
		void unregisterPluginModule(PluginModule* module);
		void registerColorScheme(const QString& schemeFilePath);
		void registerStyleTemplate(const QString& templateFilePath);
	public:
		virtual QWidget* getPluginSettingsWidget(QWidget* parent = nullptr) { return nullptr; };
	protected:
		Visindigo::__Private__::PluginPrivate* d;
	};
}

#define Visindigo_PluginMain_Function_Name "VisindigoPluginMain"
typedef Visindigo::General::Plugin* (*__VisindigoPluginMain)(void);

#define VIPlugin(PluginClass)\
public:\
	static PluginClass* getInstance();
#define VIPluginInstance(PluginClass) PluginClass::getInstance()

#endif // Visindigo_General_Plugin_H