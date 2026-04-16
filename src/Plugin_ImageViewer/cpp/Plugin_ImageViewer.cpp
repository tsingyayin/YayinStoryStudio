#include "Plugin_ImageViewer.h"
#include "YSS/ImageViewerFileServer.h"
namespace YSS::ImageViewer {
	Translator::Translator(Visindigo::General::Plugin* parent) :
		Visindigo::General::Translator(parent, "ImageViewer")
	{
		setDefaultLang(zh_CN);
		addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.imageviewer/i18n/zh_CN.json");
		addLangFilePath(en_US, ":/resource/cn.yxgeneral.imageviewer/i18n/en_US.json");
	}

	Main::Main() : YSSCore::Editor::EditorPlugin() {
		setPluginVersion(Compiled_VIAPI_Version);
		setPluginID("cn.yxgeneral.imageviewer");
		setPluginName("Image Viewer");
		setPluginAuthor({ "Tsing Yayin" });
	}
	Main::~Main() {
	}
	void Main::onPluginEnable() {
		registerPluginModule(new Translator(this));
		registerFileServer(new YSS::ImageViewer::FS_ImageViewer(this));
	}
	void Main::onApplicationInit() {
	}
	void Main::onPluginDisable() {
	}
	void Main::onTest() {
	}
	void Main::onProjectOpen(YSSCore::General::YSSProject* project) {
	}
	void Main::onProjectClose(YSSCore::General::YSSProject* project) {
	}
}