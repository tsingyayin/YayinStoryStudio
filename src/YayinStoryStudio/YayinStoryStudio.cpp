#include "YayinStoryStudio.h"
#include <Editor/FileServerManager.h>
#include <Editor/ProjectTemplateManager.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/LangServerManager.h>
#include <Utility/ExtTool.h>
#include <Utility/FileUtility.h>
#include "Editor/GlobalValue.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/ProjectPage/ProjectWin.h"
#include <General/VIApplication.h>
#include "Editor/YSSCommandHandler.h"
#include "Editor/YSSTranslator.h"

namespace YSS {
	Main::Main() {
		setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginID("cn.yxgeneral.weavestudio.yss");
		setPluginName("Yayin Story Studio");
		setPluginAuthor({ "Tsing Yayin" });
	}

	void Main::onPluginEnable() {
		VISTM->setStyleTemplatePriority({"YSS"});
		VISTM->setColorSchemePriority({ "YSS", "#Default" }); // NOTE: YSS does not have color scheme yet, this is for future use
		VISTM->setAnimationDuration(500);
		YSSCore::Editor::FileServerManager::getInstance();
		YSSCore::Editor::ProjectTemplateManager::getInstance();
		YSSCore::Editor::FileTemplateManager::getInstance();
		YSSCore::Editor::LangServerManager::getInstance();
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("yst", "YSS StyleSheet Template",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,0");
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("ysp", "YSS Plugin Library",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,1");
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("yssp", "YSS Project",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,2");
		YSS::GlobalValue::getInstance();
		VIApp->setGlobalFont(":/resource/cn.yxgeneral.yayinstorystudio/HarmonyOS_Sans_SC_Regular.ttf");
		registerPluginModule(new YSS::Editor::YSSCommandHandler(this));
		registerPluginModule(new YSS::Editor::YSSTranslator(this));
	}

	void Main::onApplicationInit() {
		YSS::ProjectPage::ProjectWin* win = new YSS::ProjectPage::ProjectWin();
		win->show();
		YSS::Editor::MainWin* mainWin = new YSS::Editor::MainWin();
		GlobalValue::setMainWindow(mainWin);
	}

	void Main::onPluginDisable() {
		GlobalValue::saveConfig();
		// Plugin disabled actions
	}

	void Main::onTest() {

		
	}
	Main::~Main() {

	}
}