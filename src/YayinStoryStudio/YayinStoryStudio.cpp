#include <QtCore/qdir.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qtimer.h>
#include <Editor/ColorThemeProvider.h>
#include <Editor/FileServerManager.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/LangServerManager.h>
#include <Editor/ProjectTemplateManager.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <Utility/BenchmarkTimer.h>
#include <Utility/Console.h>
#include <Utility/ExtTool.h>
#include <Utility/FileUtility.h>
#include <Utility/SevenZipBinder.h>
#include <Widgets/ConfigWidget.h>
#include <Widgets/Terminal.h>
#include "Editor/InstallerClient.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/ProjectPage/ProjectWin.h"
#include "Editor/YSSCommandHandler.h"
#include "Editor/YSSTranslator.h"
#include "YayinStoryStudio.h"
namespace YSS {
	class MainPrivate {
		friend class Main;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
		static Main* Instance;
	};
	Main* MainPrivate::Instance = nullptr;

	Main::Main(): Visindigo::General::Plugin("cn.yxgeneral.yayinstorystudio") {
		d = new MainPrivate;
		MainPrivate::Instance = this;
		setTestEnable();
		setPluginVersion(getPluginAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginName("Yayin Story Studio");
		setPluginAuthor({ "Tsing Yayin" });
		registerColorScheme(":/resource/cn.yxgeneral.yayinstorystudio/vst/editorTheme.json");
#ifdef Q_OS_ANDROID
		try {
#endif
			VIApp->setGlobalFont(":/resource/cn.yxgeneral.yayinstorystudio/HarmonyOS_Sans_SC_Regular.ttf");
			VIApp->setIconFont(":/resource/cn.yxgeneral.visindigo/Segoe Fluent Icons.ttf");
#ifdef Q_OS_ANDROID
		} catch (...) {
			vgWarningF << "Bundled fonts unavailable on Android; falling back to system font.";
		}
#endif
	}

	void Main::onPluginEnable() {
#ifndef Q_OS_ANDROID
		releaseInstaller();
		YSS::Editor::InstallerClient* installerClient = new YSS::Editor::InstallerClient();
		connect(installerClient, &YSS::Editor::InstallerClient::installerRequestProgramClose, this, []() {
			qApp->quit();
			});
		connect(installerClient, &YSS::Editor::InstallerClient::connected, this, []() {
			vgDebug << "Connected to installer.";
			});
		YSS::Editor::InstallerClient::getInstance()->connectToInstaller();
#endif
		auto LangID = Visindigo::General::Translator::stringToLangID(getPluginConfig()->getString("Settings.General.Language"));
		VITRH->setLangID(LangID);
		VISTM->setAnimationDuration(500);
		YSSCore::Editor::FileServerManager::getInstance();
		YSSCore::Editor::ProjectTemplateManager::getInstance();
		YSSCore::Editor::FileTemplateManager::getInstance();
		YSSCore::Editor::LangServerManager::getInstance();
#ifndef Q_OS_ANDROID
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("vst", "Visindigo StyleSheet Template",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,0");
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("vpl", "Visindigo Plugin Library",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,1");
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("yssp", "YayinStoryStudio Project",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,2");
#endif
		
		registerPluginModule(new YSS::Editor::YSSCommandHandler(this));
		registerPluginModule(new YSS::Editor::YSSTranslator(this));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget();
		d->ConfigWidget->loadCWJson(Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.yayinstorystudio/configWidget/programConfig.json"));
		d->ConfigWidget->setTargetConfig(getPluginFolder().filePath("config.json"));
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::comboBoxIndexChanged, this, [](const QString& node, int index, QString data) {
			vgDebug << node;
			if (node == "General.Theme") {
				VISTM->changeColorTheme(data);
			}
			else if (node == "General.UpdateChannel") {
				Visindigo::Utility::FileUtility::deleteFile(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_cache"));
			}
			});
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::saved, this, &Visindigo::General::Plugin::reloadPluginConfig);
		vgDebug << getPluginFolder().filePath("config.json");
		VISTM->setStyleTemplatePriority({ "YSS" });
		VISTM->setColorSchemePriority({ "YSSEditor", "#Default" }); // NOTE: YSS does not have color scheme yet, this is for future us
	}

	void Main::onApplicationInit() {
		VISTM->changeColorTheme(getPluginConfig()->getString("Settings.General.Theme"));
		YSS::ProjectPage::ProjectWin* win = new YSS::ProjectPage::ProjectWin();
		win->show();
	}

	void Main::onPluginDisable() {
		savePluginConfig();
	}

	void Main::onTest() {

	}

	void Main::releaseInstaller(){
		QString installerPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + 
		"/AppData/Local/TsingYayin/YayinStoryStudio/Installer/YSSInstaller.exe";
		bool needToRelease = false;
		if (not Visindigo::Utility::FileUtility::isFileExist(installerPath)) {
			vgDebug << "Installer not found at:" << installerPath;
			vgDebug << "Releasing installer...";
			needToRelease = true;
		}
		else {
			vgDebug << "Installer found at:" << installerPath;
			QDateTime installerLastModified = Visindigo::Utility::FileUtility::getFileModifyTime(installerPath);
			QDateTime currentInstallerLastModified = 
				Visindigo::Utility::FileUtility::getFileModifyTime(Visindigo::Utility::FileUtility::getProgramPath() + "/YSSInstaller.exe");
			if (installerLastModified < currentInstallerLastModified) {
				vgDebug << "Installer is outdated, releasing new version...";
				needToRelease = true;
			}
			else {
				vgDebug << "Installer is up to date.";
			}
		}
		if (needToRelease) {
			YSS::Editor::InstallerClient::releaseInstaller();
		}
	}
	QWidget* Main::getConfigWidget() {
		return d->ConfigWidget;
	}

	Main::~Main() {
		delete d;
	}

	Main* Main::getInstance() {
		return MainPrivate::Instance;
	}

	TestDragWidget::TestDragWidget(QWidget* parent) :QWidget(parent) {
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