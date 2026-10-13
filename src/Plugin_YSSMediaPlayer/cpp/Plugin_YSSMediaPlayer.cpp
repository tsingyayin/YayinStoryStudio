#include "Plugin_YSSMediaPlayer.h"
#include "YSS/MediaPlayerFileServer.h"

namespace YSS::MediaPlayer {
	MediaPlayerTranslator::MediaPlayerTranslator(Visindigo::General::Plugin* parent) :
		Visindigo::General::Translator(parent, "YSSMediaPlayer")
	{
		setDefaultLang(zh_CN);
		addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssmediaplayer/i18n/zh_CN.json");
		addLangFilePath(en, ":/resource/cn.yxgeneral.yayinstorystudio.plugin.yssmediaplayer/i18n/en.json");
	}

	Main::Main() : YSSCore::Editor::EditorPlugin("cn.yxgeneral.yayinstorystudio.plugin.yssmediaplayer") {
		setPluginVersion(Compiled_VIAPI_Version);
		setPluginName("YSS Media Player");
		setPluginAuthor({ "Tsing Yayin" });
	}
	Main::~Main() {}
	void Main::onPluginEnable() {
		registerPluginModule(new MediaPlayerTranslator(this));
		registerFileServer(new FS_MediaPlayer(this));
	}
	void Main::onApplicationInit() {}
	void Main::onPluginDisable() {}
	void Main::onTest() {}
	void Main::onProjectOpen(YSSCore::General::YSSProject* project) {}
	void Main::onProjectClose(YSSCore::General::YSSProject* project) {}
}
