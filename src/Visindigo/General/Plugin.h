#ifndef Visindigo_General_Plugin_H
#define Visindigo_General_Plugin_H
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "../Macro.h"
#include "../General/Version.h"
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
}
// Main
namespace Visindigo::General {
	class VisindigoAPI Plugin :public QObject {
		friend class PluginManager;
		friend class Visindigo::__Private__::PluginPrivate;
		Q_OBJECT;
	public:
		Plugin(Visindigo::General::Version abiVersion = Compiled_VIABI_Version, QString extensionID = "Visindigo_Base", QObject * parent = nullptr);
		virtual ~Plugin();
		virtual void onPluginEnable() {};
		virtual void onPluginDisbale() {};
		QString getPluginID() const;
		QString getPluginName() const;
		QStringList getPluginAuthor() const;
		QDir getPluginFolder() const;
		Visindigo::Utility::JsonConfig* getPluginConfig();
		QList<PluginModule*> getModules() const;
		Visindigo::General::Version getPluginVersion() const;
		Visindigo::General::Version getPluginABIVersion() const;
		QString getPluginExtensionID() const;
	protected:
		void setPluginVersion(const Visindigo::General::Version& version);
		void setPluginID(const QString& id);
		void setPluginName(const QString& name);
		void setPluginAuthor(const QStringList& author);
		void registerPluginModule(PluginModule* module);
		void registerTranslator(Visindigo::General::Translator* translator);
	public:
		virtual QWidget* getPluginSettingsWidget(QWidget* parent = nullptr) { return nullptr; };
	protected:
		Visindigo::__Private__::PluginPrivate* d;
	};
}

#define Visindigo_PluginMain_Function_Name "VisindigoPluginMain"
typedef Visindigo::General::Plugin* (*__VisindigoPluginMain)(void);

#endif // Visindigo_General_Plugin_H