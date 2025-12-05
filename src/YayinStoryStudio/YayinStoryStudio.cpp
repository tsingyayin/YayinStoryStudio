#include "YayinStoryStudio.h"
#include <Editor/FileServerManager.h>
#include <Editor/ProjectTemplateManager.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/LangServerManager.h>
#include <Utility/ExtTool.h>
#include <Utility/FileUtility.h>
#include "Editor/GlobalValue.h"
#include <QtGui/qfontdatabase.h>
#include <QtGui/qguiapplication.h>
#include "Editor/MainEditor/MainWin.h"
#include "Editor/ProjectPage/ProjectWin.h"
#include <QtWidgets/qapplication.h>
#include <Utility/CodeDiff.h>
#include <Utility/AsyncFunction.h>

namespace YSS {
	Main::Main() {
		setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginID("cn.yxgeneral.weavestudio.yss");
		setPluginName("Yayin Story Studio");
		setPluginAuthor({ "Tsing Yayin" });
	}

	void Main::onPluginEnable() {
		Visindigo::Editor::FileServerManager::getInstance();
		Visindigo::Editor::ProjectTemplateManager::getInstance();
		Visindigo::Editor::FileTemplateManager::getInstance();
		Visindigo::Editor::LangServerManager::getInstance();
		Visindigo::Utility::ExtTool::registerFileExtension("yst", "YSS StyleSheet Template",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,0");
		Visindigo::Utility::ExtTool::registerFileExtension("ysp", "YSS Plugin Library",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,1");
		Visindigo::Utility::ExtTool::registerFileExtension("yssp", "YSS Project",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,2");
		YSS::GlobalValue::getInstance();
		int fontID = QFontDatabase::addApplicationFont(":/yss/compiled/HarmonyOS_Sans_SC_Regular.ttf");
		QString hosFont = QFontDatabase::applicationFontFamilies(fontID).at(0);
		QFont font(hosFont);
		qApp->setFont(font);
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
		Visindigo::Utility::CodeDiff codeDiff;
		codeDiff.setOldCode(Visindigo::Utility::FileUtility::readLines("./resource/old.txt"));
		codeDiff.setNewCode(Visindigo::Utility::FileUtility::readLines("./resource/new.txt"));
		codeDiff.compare();
		//codeDiff.debugPrint();

		YSSAsync<int, int, int>(
			{ 3, 4 },
			[](int a, int b)->int {
				QThread::msleep(10000);
				return a + b;
			},

			[](int rtn) {
				qDebug() << rtn;
			}
		);
	}
	Main::~Main() {

	}
}