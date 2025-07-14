#include "../Plugin_ASEDevTool.h"
#include "../LangServer/AStoryLangServer.h"
#include "../FileServer/FS_AStory.h"
#include "../FileServer/FS_ASRule.h"
#include "../ASEDevTranslator.h"
#include "../ProjectTemplateProvider/ASEAStoryPTP.h"
#include "../FileTemplateProvider/ASEAStoryFTP.h"

Plugin_ASEDevTool::Plugin_ASEDevTool() {
	setPluginVersion(YSSCore::General::Version(0, 1, 0));
	setPluginID("ASEDevTool");
	setPluginName("ASE Development Tool");
	setPluginAuthor({ "Gra_dus" });
}
void Plugin_ASEDevTool::onPluginEnable() {
	registerTranslator(new ASEDevTranslator());
	registerLangServer(new AStoryLanguageServer(this));
	registerFileServer(new AStoryFileServer(this));
	registerFileServer(new ASRuleFileServer(this));
	registerProjectTemplateProvider(new ASEAStoryPTP(this));
	registerFileTemplateProvider(new ASEAStoryFTP(this));
}

void Plugin_ASEDevTool::onPluginDisbale() {
}