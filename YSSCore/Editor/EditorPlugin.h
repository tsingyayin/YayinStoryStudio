#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "../Macro.h"
#include "../General/Version.h"
// Forward declarations
class QString;
namespace YSSCore::Utility {
	class JsonConfig;
}
namespace YSSCore::__Private__ {
	class EditorPluginPrivate;
}
namespace YSSCore::General {
	class Translator;
}
namespace YSSCore::Editor {
	class LangServer;
	class DebugServer;
	class FileServer;
	class ProjectTemplateProvider;
	class EditorPluginManager;
	class EditorHelper;
	class EditorPluginModule;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI EditorPlugin :public QObject {
		friend class EditorPluginManager;
		friend class YSSCore::__Private__::EditorPluginPrivate;
		Q_OBJECT;
	public:
		EditorPlugin(YSSCore::General::Version abiVersion = Compiled_YSSABI_Version, QObject* parent = nullptr);
		virtual ~EditorPlugin();
		virtual void onPluginEnable() = 0;
		virtual void onPluginDisbale() = 0;
		QString getPluginID() const;
		QString getPluginName() const;
		QStringList getPluginAuthor() const;
		QString getPluginFolder() const;
		YSSCore::Utility::JsonConfig* getPluginConfig();
		QList<EditorPluginModule*> getModules() const;
	protected:
		void setPluginVersion(const YSSCore::General::Version& version);
		void setPluginID(const QString& id);
		void setPluginName(const QString& name);
		void setPluginAuthor(const QStringList& author);
		void registerLangServer(LangServer* server);
		void registerDebugServer(DebugServer* server);
		void registerFileServer(FileServer* server);
		void registerProjectTemplateProvider(ProjectTemplateProvider* provider);
		void registerTranslator(YSSCore::General::Translator* translator);
		void registerEditorHelper(YSSCore::Editor::EditorHelper* helper);
	public:
		virtual QWidget* getPluginSettingsWidget(QWidget* parent = nullptr) = 0;
	protected:
		YSSCore::__Private__::EditorPluginPrivate* d;
	};
}

typedef YSSCore::Editor::EditorPlugin* (*__YSSPluginDllMain)(void);