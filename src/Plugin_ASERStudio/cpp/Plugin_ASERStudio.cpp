#include "Plugin_ASERStudio.h"
#include "YSS/ASERProjectTemplate.h"
#include "YSS/ASERFIleServer.h"

namespace ASERStudio {
	ASERStudioTranslator::ASERStudioTranslator(Visindigo::General::Plugin* parent) :
		Visindigo::General::Translator(parent, "ASERStudio")
	{
		setDefaultLang(zh_CN);
		setLangFilePath({
			{zh_CN, ":/resource/cn.yxgeneral.aserstudio/i18n/zh_CN.json"},
			{en_US, ":/resource/cn.yxgeneral.aserstudio/i18n/en_US.json"},
			});
	}
	Main::Main() : YSSCore::Editor::EditorPlugin() {
		setPluginVersion(Visindigo::General::Version(2, 0, 0));
		setPluginID("cn.yxgeneral.aserstudio");
		setPluginName("ASER Studio");
		setPluginAuthor({ "Gra_dus", "Tsing Yayin" });
	}
	Main::~Main() {
	}
	void Main::onPluginEnable() {
		registerPluginModule(new ASERStudioTranslator(this));
		registerProjectTemplateProvider(new YSS::ProjectTemplate_AStoryX(this));
		registerFileServer(new YSS::FileServer_AStoryX(this));
		registerFileServer(new YSS::FileServer_ASRuleJson(this));
	}
	void Main::onApplicationInit() {
	}
	void Main::onPluginDisable() {
	}
	void Main::onTest() {
	}
}