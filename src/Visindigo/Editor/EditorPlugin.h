#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "../Macro.h"
#include "../General/Version.h"
#include "../General/Plugin.h"
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
	class VisindigoAPI EditorPlugin :public Visindigo::General::Plugin {
		friend class EditorPluginManager;
		friend class Visindigo::__Private__::EditorPluginPrivate;
		Q_OBJECT;
	public:
		EditorPlugin(Visindigo::General::Version abiVersion = Compiled_VIABI_Version, QObject* parent = nullptr);
		virtual ~EditorPlugin();
	protected:
		void registerLangServer(LangServer* server);
		void registerDebugServer(DebugServer* server);
		void registerFileServer(FileServer* server);
		void registerProjectTemplateProvider(ProjectTemplateProvider* provider);
		void registerFileTemplateProvider(FileTemplateProvider* provider);
	protected:
		Visindigo::__Private__::EditorPluginPrivate* d;
	};
}

typedef Visindigo::Editor::EditorPlugin* (*__YSSPluginDllMain)(void);