#include "../Plugin_YSSFileExt.h"
#include "../LangServer/JsonLangServer.h"
Plugin_YSSFileExt::Plugin_YSSFileExt() {
	setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSSFileExt uses the same version as Visindigo API version
	setPluginID("cn.yxgeneral.weavestudio.yssfileext");
	setPluginName("YSS File Extensions");
	setPluginAuthor({ "Tsing Yayin" });
}

void Plugin_YSSFileExt::onPluginEnable() {
	registerLangServer(new YSSFileExt::JsonLangServer(this));
}

void Plugin_YSSFileExt::onPluginDisbale() {

}