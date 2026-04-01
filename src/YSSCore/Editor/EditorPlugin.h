#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include "../Macro.h"
#include <General/Version.h>
#include <General/Plugin.h>
#define YSSPluginModule_LangServer "cn.yxgeneral.yss.langserver"
#define YSSPluginModule_DebugServer "cn.yxgeneral.yss.debugserver"
#define YSSPluginModule_FileServer "cn.yxgeneral.yss.fileserver"
#define YSSPluginModule_ProjectTemplateProvider "cn.yxgeneral.yss.project_template_provider"
#define YSSPluginModule_FileTemplateProvider "cn.yxgeneral.yss.file_template_provider"
#define YSSPluginTypeID "YayinStoryStudio"

// Forward declarations
class QString;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace YSSCore::__Private__ {
	class EditorPluginPrivate;
}
namespace Visindigo::General {
	class Translator;
}
namespace YSSCore::General {
	class YSSProject;
}
namespace YSSCore::Editor {
	class LangServer;
	class DebugServer;
	class FileServer;
	class ProjectTemplateProvider;
	class FileTemplateProvider;
	class EditorPluginManager;
	class EditorPluginModule;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI EditorPlugin :public Visindigo::General::Plugin {
		friend class EditorPluginManager;
		friend class YSSCore::__Private__::EditorPluginPrivate;
		Q_OBJECT;
	public:
		EditorPlugin(Visindigo::General::Version apiVersion = Compiled_VIAPI_Version, Visindigo::General::Version abiVersion = Compiled_VIABI_Version, QObject* parent = nullptr);
		virtual ~EditorPlugin();
	public:
		virtual void onProjectOpen(YSSCore::General::YSSProject* project) {};
		virtual void onProjectClose(YSSCore::General::YSSProject* project) {};
	protected:
		void registerLangServer(LangServer* server);
		void registerDebugServer(DebugServer* server);
		void registerFileServer(FileServer* server);
		void registerProjectTemplateProvider(ProjectTemplateProvider* provider);
		void registerFileTemplateProvider(FileTemplateProvider* provider);
		YSSCore::General::YSSProject* getCurrentProject();
	protected:
		YSSCore::__Private__::EditorPluginPrivate* d;
	};
}

typedef YSSCore::Editor::EditorPlugin* (*__YSSPluginDllMain)(void);