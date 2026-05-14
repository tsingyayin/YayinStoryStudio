#ifndef YSS_Editor_MainWin_MainEditorBuiltinPlugin_h
#define YSS_Editor_MainWin_MainEditorBuiltinPlugin_h
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
#endif // YSS_Editor_MainWin_MainEditorBuiltinPlugin_h