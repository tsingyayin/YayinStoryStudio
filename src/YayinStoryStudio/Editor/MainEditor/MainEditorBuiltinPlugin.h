#ifndef YayinStoryStudio_Editor_MainEditor_MainEditorBuiltinPlugin_h
#define YayinStoryStudio_Editor_MainEditor_MainEditorBuiltinPlugin_h
#include <Editor/EditorPlugin.h>
namespace YSS::Editor {
	class MainEditorBuiltinPlugin :public YSSCore::Editor::EditorPlugin {
		Q_OBJECT;
	public:
		MainEditorBuiltinPlugin();
		virtual ~MainEditorBuiltinPlugin();
		virtual void onPluginEnable() override;
		virtual QWidget* onToolWidgetRequested(const QString& widgetID) override;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_MainEditorBuiltinPlugin_h
