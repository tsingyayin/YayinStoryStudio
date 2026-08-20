#include "Plugin_YSSFileExt.h"
#include "LangServer/JsonLangServer.h"
#include "FileServer/DefaultTextEdit.h"
#include "FileServer/YSSPEditor.h"
namespace YSSFileExt {
	YSSFileExtTranslator::YSSFileExtTranslator(Visindigo::General::Plugin* parent) :
		Visindigo::General::Translator(parent, "YSSFileExt")
	{
		setDefaultLang(zh_CN);
		addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/zh_CN.json");
		addLangFilePath(zh_TW, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/zh_TW.json");
		addLangFilePath(en, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/en.json");
		addLangFilePath(ja, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/ja.json");
		addLangFilePath(jp_less_loanword, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/jp_less_loanword.json");
		addLangFilePath(ko, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/ko.json");
		addLangFilePath(ru, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/ru.json");
		addLangFilePath(de, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/de.json");
		addLangFilePath(fr, ":/resource/cn.yxgeneral.weavestudio.yssfileext/i18n/fr.json");
	}
}
Plugin_YSSFileExt::Plugin_YSSFileExt() {
	setPluginVersion(Compiled_VIAPI_Version); // YSSFileExt uses the same version as Visindigo API version
	setPluginID("cn.yxgeneral.weavestudio.yssfileext");
	setPluginName("YSS File Extensions");
	setPluginAuthor({ "Tsing Yayin" });
}

void Plugin_YSSFileExt::onPluginEnable() {
	registerPluginModule(new YSSFileExt::YSSFileExtTranslator(this));
	registerLangServer(new YSSFileExt::JsonLangServer(this));
	registerFileServer(new YSSFileExt::YSSPFileServer(this));
	registerFileServer(new YSSFileExt::DefaultTextEdit(this));
}

void Plugin_YSSFileExt::onPluginDisable() {
}