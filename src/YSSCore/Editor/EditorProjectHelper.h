#ifndef YSSCore_Editor_EditorProjectHelper_h
#define YSSCore_Editor_EditorProjectHelper_h
#include <General/PluginModule.h>
namespace YSSCore::General {
	class YSSProject;
}
namespace YSSCore::Editor {
	class EditorToolWidget;
	class EditorProjectHelper : Visindigo::General::PluginModule {
	public:
		EditorProjectHelper(const QString& moduleName, const QString& moduleID, Visindigo::General::Plugin* plugin = nullptr) {};
		virtual ~EditorProjectHelper() {};
		virtual void onModuleEnable() override {};
		virtual void onModuleDisable() override {};
		virtual void onProjectOpen(YSSCore::General::YSSProject* project) {};
		virtual void onProjectReload(YSSCore::General::YSSProject* project) {};
		virtual bool onProjectClose(YSSCore::General::YSSProject* project) { return true; };
		virtual YSSCore::Editor::EditorToolWidget* getToolWidget() { return nullptr; }
	};
}
#endif // YSSCore_Editor_EditorProjectHelper_h
