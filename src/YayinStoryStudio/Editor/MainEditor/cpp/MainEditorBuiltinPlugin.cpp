#include "Editor/MainEditor/ColorThemeSettings.h"
#include "Editor/MainEditor/DebugCommand.h"
#include "Editor/MainEditor/MainEditorBuiltinPlugin.h"
#include "Editor/MainEditor/MessageViewer.h"
#include "Editor/MainEditor/MultiTerminal.h"
#include "Editor/MainEditor/PreferenceEditWidget.h"
#include "Editor/MainEditor/ResourceBrowser.h"
namespace YSS::Editor {
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
		registerPluginModule(new DebugCommand(this));
	}
	QWidget* MainEditorBuiltinPlugin::onToolWidgetRequested(const QString& widgetID) {
		if (widgetID == "cn.yxgeneral.yss.messageViewer") {
			return new MessageViewer;
		}
		else if (widgetID == "cn.yxgeneral.yss.multiTerminal") {
			return new MultiTerminal;
		}
		return nullptr;
	}
}