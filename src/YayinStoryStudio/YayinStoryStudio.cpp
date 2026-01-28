#include "YayinStoryStudio.h"
#include <Editor/FileServerManager.h>
#include <Editor/ProjectTemplateManager.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/LangServerManager.h>
#include <Utility/ExtTool.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include "Editor/GlobalValue.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/ProjectPage/ProjectWin.h"
#include <General/VIApplication.h>
#include <Utility/GeneralConfig.h>
#include <Utility/GeneralConfigParser.h>
#include "General/YSSLogger.h"
#include <chrono>
#include "Editor/YSSCommandHandler.h"
namespace YSS {
	Main::Main() {
		setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginID("cn.yxgeneral.weavestudio.yss");
		setPluginName("Yayin Story Studio");
		setPluginAuthor({ "Tsing Yayin" });
	}

	void Main::onPluginEnable() {
		VISTM->setStyleTemplatePriority({"YSS", "#Default"});
		VISTM->setColorSchemePriority({ "YSS", "#Default" });
		VISTM->setAnimationDuration(500);
		YSSCore::Editor::FileServerManager::getInstance();
		YSSCore::Editor::ProjectTemplateManager::getInstance();
		YSSCore::Editor::FileTemplateManager::getInstance();
		YSSCore::Editor::LangServerManager::getInstance();
		Visindigo::Utility::ExtTool::registerFileExtension("yst", "YSS StyleSheet Template",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,0");
		Visindigo::Utility::ExtTool::registerFileExtension("ysp", "YSS Plugin Library",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,1");
		Visindigo::Utility::ExtTool::registerFileExtension("yssp", "YSS Project",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,2");
		YSS::GlobalValue::getInstance();
		VIApp->setGlobalFont(":/yss/compiled/HarmonyOS_Sans_SC_Regular.ttf");
		YSS::YSSCommandHandler* cmdHandler = new YSS::YSSCommandHandler();
		cmdHandler->enable();
	}

	void Main::onApplicationInit() {
		YSS::ProjectPage::ProjectWin* win = new YSS::ProjectPage::ProjectWin();
		win->show();
		YSS::Editor::MainWin* mainWin = new YSS::Editor::MainWin();
		GlobalValue::setMainWindow(mainWin);
	}

	void Main::onPluginDisbale() {
		GlobalValue::saveConfig();
		// Plugin disabled actions
	}

	void Main::onTest() {

		
	}
	Main::~Main() {

	}
}