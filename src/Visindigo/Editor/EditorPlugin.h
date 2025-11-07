#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "../Macro.h"
#include "../General/Version.h"
// Forward declarations
class QString;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::__Private__ {
	class EditorPluginPrivate;
}
namespace Visindigo::General {
	class Translator;
}
namespace Visindigo::Editor {
	class LangServer;
	class DebugServer;
	class FileServer;
	class ProjectTemplateProvider;
	class FileTemplateProvider;
	class EditorPluginManager;
	class EditorPluginModule;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI EditorPlugin :public QObject {
		friend class EditorPluginManager;
		friend class Visindigo::__Private__::EditorPluginPrivate;
		Q_OBJECT;
	public:
		EditorPlugin(Visindigo::General::Version abiVersion = Compiled_VIABI_Version, QObject* parent = nullptr);
		virtual ~EditorPlugin();
		virtual void onPluginEnable() = 0;
		virtual void onPluginDisbale() = 0;
		QString getPluginID() const;
		QString getPluginName() const;
		QStringList getPluginAuthor() const;
		QString getPluginFolder() const;
		Visindigo::Utility::JsonConfig* getPluginConfig();
		QList<EditorPluginModule*> getModules() const;
	protected:
		void setPluginVersion(const Visindigo::General::Version& version);
		void setPluginID(const QString& id);
		void setPluginName(const QString& name);
		void setPluginAuthor(const QStringList& author);
		void registerLangServer(LangServer* server);
		void registerDebugServer(DebugServer* server);
		void registerFileServer(FileServer* server);
		void registerProjectTemplateProvider(ProjectTemplateProvider* provider);
		void registerFileTemplateProvider(FileTemplateProvider* provider);
		void registerTranslator(Visindigo::General::Translator* translator);
	public:
		virtual QWidget* getPluginSettingsWidget(QWidget* parent = nullptr) = 0;
	protected:
		Visindigo::__Private__::EditorPluginPrivate* d;
	};
}

typedef Visindigo::Editor::EditorPlugin* (*__YSSPluginDllMain)(void);