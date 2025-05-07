#include "Plugin_AStory.h"
#include "LangServer/LS_AStory.h"
#include "FileServer/FS_AStory.h"
#include "FileServer/FS_ASRule.h"
Plugin_AStory::Plugin_AStory() {
	setPluginVersion(YSSCore::General::Version(0, 1, 0));
	setPluginID("AStory");
	setPluginName("ASE Develop Environment");
	setPluginAuthor({ "Gra_dus" });
}
void Plugin_AStory::onPluginEnable() {
	registerLangServer(new AStoryLanguageServer());
	registerFileServer(new AStoryFileServer());
	registerFileServer(new ASRuleFileServer());
}

void Plugin_AStory::onPluginDisbale() {
}