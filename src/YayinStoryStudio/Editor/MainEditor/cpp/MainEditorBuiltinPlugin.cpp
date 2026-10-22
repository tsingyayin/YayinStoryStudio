#include "Editor/MainEditor/ColorThemeSettings.h"
#include "Editor/MainEditor/DebugCommand.h"
#include "Editor/MainEditor/MainEditorBuiltinPlugin.h"
#include "Editor/MainEditor/MessageViewer.h"
#include "Editor/MainEditor/MultiTerminal.h"
#include "Editor/MainEditor/PreferenceEditWidget.h"
#include "Editor/MainEditor/ResourceBrowser.h"
#include <Editor/CodeDiff.h>
namespace YSS::Editor {
	// 比较器的虚拟文件服务：只负责创建 CodeDiff，参数由虚拟文件路径携带。
	class CodeDiffVirtualFileServer : public YSSCore::Editor::FileServer {
	public:
		CodeDiffVirtualFileServer(YSSCore::Editor::EditorPlugin* plugin) :
			YSSCore::Editor::FileServer("YSS Built-in Code Diff", "cn.yxgeneral.yss_builtin.codeDiffVFS", plugin) {
			setEditorType(YSSCore::Editor::FileServer::BuiltInEditor);
			setSupportedFileExts({ YSSVirtualFile_CodeDiff });
			setAsVitrualFileServer(true);
			setListAsTool(false);
			setPreferredOrientation(YSSCore::Editor::FileServer::Horizontal);
		}
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override {
			return new YSSCore::Editor::CodeDiff();
		}
	};

	MainEditorBuiltinPlugin::MainEditorBuiltinPlugin():YSSCore::Editor::EditorPlugin("cn.yxgeneral.yayinstorystudio.builtin.maineditor") {
		setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginName("YSS MainEditor Builtin");
		setPluginAuthor({ "Tsing Yayin" });
	}
	MainEditorBuiltinPlugin::~MainEditorBuiltinPlugin() {}
	void MainEditorBuiltinPlugin::onPluginEnable() {
		//registerToolWidget("cn.yxgeneral.yss.messageViewer", "i18n:YSS::editor.messageViewer.title");
		//registerToolWidget("cn.yxgeneral.yss.multiTerminal", "i18n:YSS::editor.multiTerminal.title");
		registerFileServer(new MessageViewerVFS(this));
		registerFileServer(new MultiTerminalVFS(this));
		registerFileServer(new ResourceBrowserVFS(this));
		registerFileServer(new PreferenceVFServer(this));
		registerFileServer(new ColorThemeSettingsVFServer(this));
		registerFileServer(new CodeDiffVirtualFileServer(this));
		registerPluginModule(new DebugCommand(this));
	}
}