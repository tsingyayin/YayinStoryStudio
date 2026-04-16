#include "Plugin_YSSFileExt.h"
#include "LangServer/JsonLangServer.h"
#include "FileServer/YSSPEditor.h"
Plugin_YSSFileExt::Plugin_YSSFileExt() {
	setPluginVersion(Compiled_VIAPI_Version); // YSSFileExt uses the same version as Visindigo API version
	setPluginID("cn.yxgeneral.weavestudio.yssfileext");
	setPluginName("YSS File Extensions");
	setPluginAuthor({ "Tsing Yayin" });
}

void Plugin_YSSFileExt::onPluginEnable() {
	registerLangServer(new YSSFileExt::JsonLangServer(this));
	registerFileServer(new YSSFileExt::YSSPFileServer(this));
}

void Plugin_YSSFileExt::onPluginDisable() {

}