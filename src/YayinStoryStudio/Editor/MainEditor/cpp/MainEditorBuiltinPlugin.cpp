#include "Editor/MainEditor/MainEditorBuiltinPlugin.h"
#include "Editor/MainEditor/MessageViewer.h"
#include "Editor/MainEditor/MultiTerminal.h"
#include "Editor/MainEditor/PreferenceEditWidget.h"
#include "Editor/MainEditor/ColorThemeSettings.h"
#include "Editor/MainEditor/DebugCommand.h"
namespace YSS::Editor {
	MainEditorBuiltinPlugin::MainEditorBuiltinPlugin() {
		setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginID("cn.yxgeneral.yss_builtin");
		setPluginName("YSS MainEditor Builtin");
		setPluginAuthor({ "Tsing Yayin" });
	}
	MainEditorBuiltinPlugin::~MainEditorBuiltinPlugin() {}
	void MainEditorBuiltinPlugin::onPluginEnable() {
		//registerToolWidget("cn.yxgeneral.yss.messageViewer", "i18n:YSS::editor.messageViewer.title");
		//registerToolWidget("cn.yxgeneral.yss.multiTerminal", "i18n:YSS::editor.multiTerminal.title");
		registerFileServer(new MessageViewerVFS(this));
		registerFileServer(new MultiTerminalVFS(this));
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