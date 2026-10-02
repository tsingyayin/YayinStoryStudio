#include <QtGui/qimagereader.h>
#include "Plugin_ImageViewer.h"
#include "YSS/ImageViewerFileServer.h"
namespace YSS::ImageViewer {
	static constexpr int MinImageAllocationLimitMB = 512;

	static void ensureImageAllocationLimit() {
		if (QImageReader::allocationLimit() < MinImageAllocationLimitMB) {
			QImageReader::setAllocationLimit(MinImageAllocationLimitMB);
		}
	}

	Translator::Translator(Visindigo::General::Plugin* parent) :
		Visindigo::General::Translator(parent, "ImageViewer")
	{
		setDefaultLang(zh_CN);
		addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.imageviewer/i18n/zh_CN.json");
		addLangFilePath(en, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.imageviewer/i18n/en.json");
	}

	Main::Main() : YSSCore::Editor::EditorPlugin("cn.yxgeneral.yayinstorystudio.plugin.imageviewer") {
		setPluginVersion(Compiled_VIAPI_Version);
		setPluginName("Image Viewer");
		setPluginAuthor({ "Tsing Yayin" });
	}
	Main::~Main() {}
	void Main::onPluginEnable() {
		ensureImageAllocationLimit();
		registerPluginModule(new Translator(this));
		registerFileServer(new YSS::ImageViewer::FS_ImageViewer(this));
	}
	void Main::onApplicationInit() {}
	void Main::onPluginDisable() {}
	void Main::onTest() {}
	void Main::onProjectOpen(YSSCore::General::YSSProject* project) {}
	void Main::onProjectClose(YSSCore::General::YSSProject* project) {}
}