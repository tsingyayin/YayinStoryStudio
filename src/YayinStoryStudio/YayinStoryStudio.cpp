#include "YayinStoryStudio.h"
#include <Editor/FileServerManager.h>
#include <Editor/ProjectTemplateManager.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/LangServerManager.h>
#include <Utility/ExtTool.h>
#include <Utility/FileUtility.h>
#include "Editor/MainEditor/MainWin.h"
#include "Editor/ProjectPage/ProjectWin.h"
#include <General/VIApplication.h>
#include "Editor/YSSCommandHandler.h"
#include "Editor/YSSTranslator.h"
#include <General/Log.h>
#include <Utility/FileUtility.h>
#include <Widgets/ConfigWidget.h>
#include <QtCore/qdir.h>
#include <General/TranslationHost.h>
#include <Utility/BenchmarkTimer.h>
#include <Utility/SevenZipBinder.h>
#include <Widgets/Terminal.h>
#include <QtCore/qstandardpaths.h>
#include "Utility/Console.h"
#include "Editor/InstallerClient.h"
#include <QtCore/qtimer.h>
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
		//setTestEnable();
		setPluginVersion(getPluginAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginID("cn.yxgeneral.yayinstorystudio");
		setPluginName("Yayin Story Studio");
		setPluginAuthor({ "Tsing Yayin" });
		registerColorScheme(":/resource/cn.yxgeneral.yayinstorystudio/vst/editorTheme.json");
		VIApp->setGlobalFont(":/resource/cn.yxgeneral.yayinstorystudio/HarmonyOS_Sans_SC_Regular.ttf");
		VIApp->setIconFont(":/resource/cn.yxgeneral.visindigo/Segoe Fluent Icons.ttf");
	}

	void Main::onPluginEnable() {
		releaseInstaller();
		YSS::Editor::InstallerClient* installerClient = new YSS::Editor::InstallerClient();
		connect(installerClient, &YSS::Editor::InstallerClient::installerNotLaunched, this, [this]() {
			QString installerPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
				"/AppData/Local/TsingYayin/YayinStoryStudio/Installer/YSSInstaller.exe";
			QString launchCommand = QString("start \"\" \"%1\"").arg(installerPath);
			Visindigo::Utility::Console::exec(launchCommand);
			QTimer::singleShot(5000, this, [this]() {
				YSS::Editor::InstallerClient::getInstance()->connectToInstaller();
				});
			});
		connect(installerClient, &YSS::Editor::InstallerClient::installerRequestProgramClose, this, []() {
			qApp->quit();
			});
		connect(installerClient, &YSS::Editor::InstallerClient::connected, this, []() {
			vgDebug << "Connected to installer.";
			});
		YSS::Editor::InstallerClient::getInstance()->connectToInstaller();
		auto LangID = Visindigo::General::Translator::stringToLangID(getPluginConfig()->getString("Settings.General.Language"));
		VITRH->setLangID(LangID);
		VISTM->setAnimationDuration(500);
		YSSCore::Editor::FileServerManager::getInstance();
		YSSCore::Editor::ProjectTemplateManager::getInstance();
		YSSCore::Editor::FileTemplateManager::getInstance();
		YSSCore::Editor::LangServerManager::getInstance();
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("vst", "Visindigo StyleSheet Template",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,0");
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("vpl", "Visindigo Plugin Library",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,1");
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("yssp", "YayinStoryStudio Project",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,2");
		
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
		auto t1 = 0;
		auto t2 = 0;
		{
			auto timer = Visindigo::Utility::BenchmarkTimer();
			for (int i = 0; i < 10000;i++) {
				vgDebug << "Test" << i;
			}
			t1 = timer.elapsed();
		}
		{
			auto timer = Visindigo::Utility::BenchmarkTimer();
			for (int i = 0; i < 10000; i++) {
				qDebug() << "Test" << i;
			}
			t2 = timer.elapsed();
		}
		vgDebug << "Visindigo Logger:" << t1 << "ms";
		vgDebug << "Qt Logger:" << t2 << "ms"; 
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
		QString installerFolder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
		"/AppData/Local/TsingYayin/YayinStoryStudio/Installer";
		QStringList files = {
			"Visindigo.dll", "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6Network.dll", "Qt6Sql.dll",
			"Qt6Svg.dll", "dbghelp.dll", "icuuc.dll", "opengl32sw.dll", "7za.exe", "YSSInstaller.exe"
		};
		QStringList folders = {
			"iconengiens", "imageformats", "networkinformation", "platforms", "styles", "translations"
		};
		Visindigo::Utility::FileUtility::createDir(installerFolder);
		for (const QString& file : files) {
			Visindigo::Utility::FileUtility::copyFile(Visindigo::Utility::FileUtility::getProgramPath() + 
			"/" + file, installerFolder + "/" + file, true, true);
		}
		for (const QString& folder : folders) {
			Visindigo::Utility::FileUtility::copyDir(Visindigo::Utility::FileUtility::getProgramPath() + 
			"/" + folder, installerFolder + "/" + folder, true, true);
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