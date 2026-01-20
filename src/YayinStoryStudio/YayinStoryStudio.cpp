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
namespace YSS {
	Main::Main() {
		setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginID("cn.yxgeneral.weavestudio.yss");
		setPluginName("Yayin Story Studio");
		setPluginAuthor({ "Tsing Yayin" });
	}

	void Main::onPluginEnable() {
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
	}

	void Main::onApplicationInit() {
		YSS::ProjectPage::ProjectWin* win = new YSS::ProjectPage::ProjectWin();
		win->show();
		YSS::Editor::MainWin* mainWin = new YSS::Editor::MainWin();
		GlobalValue::setMainWindow(mainWin);
	}

	void Main::onPluginDisbale() {
		// Plugin disabled actions
	}

	void Main::onTest() {
		QString jsonStr = Visindigo::Utility::FileUtility::readAll(":/yss/compiled/configWidget/mainEditorMenu.json");
		auto start = std::chrono::high_resolution_clock::now();
		Visindigo::Utility::GeneralConfig* configt = Visindigo::Utility::GeneralConfigParser::parseFromJson_2(jsonStr);
		QString output = Visindigo::Utility::GeneralConfigParser::serializeToJson(configt, Visindigo::Utility::GeneralConfig::StringFormat::Formatted, 4);
		delete configt;
		yDebug << output;
		for (int i = 0; i < 1000; i++) {
			Visindigo::Utility::GeneralConfig* config = Visindigo::Utility::GeneralConfigParser::parseFromJson_2(jsonStr);
			delete config;
		}
		auto end = std::chrono::high_resolution_clock::now();
		yDebug << "GeneralConfig 1000次 构造、解析和析构耗时 " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms";
		Visindigo::Utility::GeneralConfig* config = Visindigo::Utility::GeneralConfigParser::parseFromJson(jsonStr);
		start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 1000; i++) {
			config->setString("测试字符串", "menu.0.Actions.0.Actions.0.ID");
		}
		end = std::chrono::high_resolution_clock::now();
		yDebug << "GeneralConfig 1000次 七层写耗时 " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " micro seconds";
		start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 1000; i++) {
			QString val = config->getString("menu.0.Actions.0.Actions.0.ID");
		}
		end = std::chrono::high_resolution_clock::now();
		yDebug << "GeneralConfig 1000次 七层读耗时 " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " micro seconds";
		start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 1000; i++) {
			QString output = Visindigo::Utility::GeneralConfigParser::serializeToJson(config, Visindigo::Utility::GeneralConfig::StringFormat::Formatted, 4);
		}
		end = std::chrono::high_resolution_clock::now();
		yDebug << "GeneralConfig 1000次 序列化耗时 " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms";
		
		auto jstart = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 1000; i++) {
			Visindigo::Utility::JsonConfig* jsonConfig = new Visindigo::Utility::JsonConfig();
			jsonConfig->parse(jsonStr);
			delete jsonConfig;
		}
		auto jend = std::chrono::high_resolution_clock::now();
		yDebug << "JsonConfig 1000次 构造、解析和析构耗时 " << std::chrono::duration_cast<std::chrono::milliseconds>(jend - jstart).count() << " ms";

		Visindigo::Utility::JsonConfig* jsonConfig = new Visindigo::Utility::JsonConfig();
		jsonConfig->parse(jsonStr);
		jstart = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 1000; i++) {
			jsonConfig->setString("测试字符串", "menu.0.Actions.0.Actions.0.ID");
		}
		jend = std::chrono::high_resolution_clock::now();
		yDebug << "JsonConfig 1000次 七层写耗时 " << std::chrono::duration_cast<std::chrono::microseconds>(jend - jstart).count() << " micro seconds";

		jstart = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 1000; i++) {
			QString val = jsonConfig->getString("menu.0.Actions.0.Actions.0.ID");
		}
		jend = std::chrono::high_resolution_clock::now();
		yDebug << "JsonConfig 1000次 七层读耗时 " << std::chrono::duration_cast<std::chrono::microseconds>(jend - jstart).count() << " micro seconds";
		jstart = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 1000; i++) {
			QString output = jsonConfig->toString( QJsonDocument::JsonFormat::Indented);
		}
		jend = std::chrono::high_resolution_clock::now();
		yDebug << "JsonConfig 1000次 序列化耗时 " << std::chrono::duration_cast<std::chrono::milliseconds>(jend - jstart).count() << " ms";
		
	}
	Main::~Main() {

	}
}