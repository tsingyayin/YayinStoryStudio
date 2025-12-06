#include "../Plugin_ASEDevTool.h"
#include "../LangServer/AStoryLangServer.h"
#include "../FileServer/FS_AStory.h"
#include "../FileServer/FS_ASRule.h"
#include "../ASEDevTranslator.h"
#include "../ProjectTemplateProvider/ASEAStoryPTP.h"
#include "../FileTemplateProvider/ASEAStoryFTP.h"
#include "../DebugServer/DS_ASE.h"
Plugin_ASEDevTool::Plugin_ASEDevTool() {
	setPluginVersion(Visindigo::General::Version(0, 1, 0));
	setPluginID("cn.yxgeneral.studioindigo.ase_dev_tool");
	setPluginName("ASE Development Tool");
	setPluginAuthor({ "Gra_dus", "Tsing Yayin"});
}
void Plugin_ASEDevTool::onPluginEnable() {
	registerTranslator(new ASEDevTranslator());
	registerLangServer(new AStoryLanguageServer(this));
	registerFileServer(new AStoryFileServer(this));
	registerFileServer(new ASRuleFileServer(this));
	registerProjectTemplateProvider(new ASEAStoryPTP(this));
	registerFileTemplateProvider(new ASEAStoryFTP(this));
	registerDebugServer(new DS_ASE(this));
}

void Plugin_ASEDevTool::onPluginDisbale() {
}