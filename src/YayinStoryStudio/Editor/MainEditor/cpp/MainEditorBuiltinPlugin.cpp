#include "Editor/MainEditor/MainEditorBuiltinPlugin.h"
#include "Editor/MainEditor/MessageViewer.h"
namespace YSS::Editor {
	MainEditorBuiltinPlugin::MainEditorBuiltinPlugin() {
		setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginID("cn.yxgeneral.yss_builtin");
		setPluginName("YSS MainEditor Builtin");
		setPluginAuthor({ "Tsing Yayin" });
	}
	MainEditorBuiltinPlugin::~MainEditorBuiltinPlugin() {}
	void MainEditorBuiltinPlugin::onPluginEnable() {
		registerToolWidget("cn.yxgeneral.yss.messageViewer", "i18n:YSS::editor.messageViewer.title");
	}
	QWidget* MainEditorBuiltinPlugin::onToolWidgetRequested(const QString& widgetID) {
		if (widgetID == "cn.yxgeneral.yss.messageViewer") {
			return new MessageViewer;
		}
		return nullptr;
	}
}