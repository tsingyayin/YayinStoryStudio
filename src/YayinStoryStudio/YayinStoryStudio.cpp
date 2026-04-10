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
#include <General/Log.h>
#include <Utility/FileUtility.h>
#include <Widgets/ConfigWidget.h>
#include <QtCore/qdir.h>
namespace YSS {
	class MainPrivate {
		friend class Main;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
		static Main* Instance;
	};
	Main* MainPrivate::Instance = nullptr;

	Main::Main() {
		d = new MainPrivate;
		MainPrivate::Instance = this;
		setPluginVersion(Visindigo::General::Version::getAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginID("cn.yxgeneral.yayinstorystudio");
		setPluginName("Yayin Story Studio");
		setPluginAuthor({ "Tsing Yayin" });
		registerColorScheme(":/resource/cn.yxgeneral.yayinstorystudio/vst/editorTheme.json");
		
	}

	void Main::onPluginEnable() {
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
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget();
		d->ConfigWidget->loadCWJson(Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.yayinstorystudio/configWidget/programConfig.json"));
		d->ConfigWidget->setTargetConfig(getPluginFolder().filePath("config.json"));
		vgDebug << getPluginFolder().filePath("config.json");
		d->ConfigWidget->setIndependentMode(true);
		VISTM->setStyleTemplatePriority({ "YSS" });
		VISTM->setColorSchemePriority({ "YSSEditor", "#Default" }); // NOTE: YSS does not have color scheme yet, this is for future use
		VISTM->setAutoAdjustThemeToSystem(false);
		
	}

	void Main::onApplicationInit() {
		VISTM->changeColorTheme("Dark");
		YSS::ProjectPage::ProjectWin* win = new YSS::ProjectPage::ProjectWin();
		win->show();
		//YSS::TestDragWidget* tw = new YSS::TestDragWidget();
		//tw->show();
	}

	void Main::onPluginDisable() {
		GlobalValue::saveConfig();
		// Plugin disabled actions
	}

	void Main::onTest() {

		
	}

	QWidget* Main::getConfigWidget() {
		return d->ConfigWidget;
	}

	Main::~Main() {

	}

	Main* Main::getInstance() {
		return MainPrivate::Instance;
	}

	TestDragWidget::TestDragWidget(QWidget* parent):QWidget(parent) {
		DragArea = new Visindigo::Widgets::DragWidget(this);
		Label1 = new QLabel("Test1", this);
		Label2 = new QLabel("Test2", this);
		Label3 = new QLabel("Test3", this);
		Label4 = new QLabel("Test4", this);
		DragArea->addWidget(Label1);
		DragArea->addWidget(Label2);
		DragArea->addWidget(Label3);
		DragArea->addWidget(Label4);
		this->setStyleSheet(R"(
	QWidget{
		border: 1px solid white;
	}
)");
	}
	void TestDragWidget::resizeEvent(QResizeEvent* event) {
		DragArea->setGeometry(0, 0, width(), height());
	}
}