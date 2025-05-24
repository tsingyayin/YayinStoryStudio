#include "../Plugin_ASEDevTool.h"
#include "../LangServer/LS_AStory.h"
#include "../FileServer/FS_AStory.h"
#include "../FileServer/FS_ASRule.h"
#include "../ASEDevTranslator.h"
Plugin_ASEDevTool::Plugin_ASEDevTool() {
	setPluginVersion(YSSCore::General::Version(0, 1, 0));
	setPluginID("ASEDevTool");
	setPluginName("ASE Development Tool");
	setPluginAuthor({ "Gra_dus" });
}
void Plugin_ASEDevTool::onPluginEnable() {
	registerTranslator(new ASEDevTranslator());
	registerLangServer(new AStoryLanguageServer());
	registerFileServer(new AStoryFileServer());
	registerFileServer(new ASRuleFileServer());
}

void Plugin_ASEDevTool::onPluginDisbale() {
}