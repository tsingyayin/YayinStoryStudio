#include "../MainWin.h"
#include "../FileEditorArea.h"
#include "../ResourceBrowser.h"
#include "../../GlobalValue.h"
#include <Utility/JsonConfig.h>
#include <Widgets/ThemeManager.h>
#include <Widgets/QuickMenu.h>
#include <Utility/FileUtility.h>
#include <Editor/FileServerManager.h>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QSplitter>
#include <General/TranslationHost.h>
#include <General/YSSProject.h>
#include "../MenuBarHandler.h"
namespace YSS::Editor {
	MainWin::MainWin() :QMainWindow() {
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		int width = GlobalValue::getConfig()->getInt("Window.Editor.Width");
		int height = GlobalValue::getConfig()->getInt("Window.Editor.Height");
		this->setMinimumSize(800, 600);
		this->resize(width, height);
		this->setWindowTitle("Yayin Story Studio");
		if (GlobalValue::getConfig()->getBool("Window.Editor.Maximized")) {
			this->showMaximized();
		}
		//ProxyWidget->setParent(this);
		CentralWidget = new QWidget(this);
		Layout = new QHBoxLayout(CentralWidget);
		QSplitter* splitter = new QSplitter(Qt::Horizontal, CentralWidget);
		Editor = new FileEditorArea(CentralWidget);
		Browser = new ResourceBrowser(CentralWidget);
		Browser->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
		Editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		Layout->addWidget(splitter);
		splitter->addWidget(Browser);
		splitter->addWidget(Editor);

		connect(YSSFSM, &Visindigo::Editor::FileServerManager::builtinEditorCreated,
			Editor, &FileEditorArea::addFileEditWidget);

		this->setCentralWidget(CentralWidget);
		this->initMenu();
		this->applyStyleSheet();
		GlobalValue::getCurrentProject()->refreshLastModifyTime();
		GlobalValue::getCurrentProject()->saveProject();
		QStringList openedFiles = GlobalValue::getCurrentProject()->getEditorOpenedFiles();
		for (const QString& filePath : openedFiles) {
			YSSFSM->openFile(filePath);
		}
		QString focusedFile = GlobalValue::getCurrentProject()->getFocusedFile();
		Editor->focusOn(focusedFile);
	}

	void MainWin::initMenu() {
		Menu = new Visindigo::Widgets::QuickMenu(this);
		Menu->setActionHandler(new MenuActionHandler(this));
		Menu->loadFromJson(Visindigo::Utility::FileUtility::readAll(":/yss/compiled/configWidget/mainEditorMenu.json"));
		this->setMenuBar(Menu);
		MenuBar = new QMenuBar(this);
		//this->setMenuBar(MenuBar);
		FileMenu = MenuBar->addMenu(YSSTR("Visindigo::general.file"));
		SaveAction = new QAction(YSSTR("Visindigo::general.save"), this);
		SaveAsAction = new QAction(YSSTR("Visindigo::general.saveAs"), this);
		FileMenu->addAction(SaveAction);
		FileMenu->addAction(SaveAsAction);
	}

	void MainWin::applyStyleSheet() {
		this->setStyleSheet("QWidget{\
			background-color: " % YSSTM->getColorString("ThemeColor.Background") % ";\
			color: " % YSSTM->getColorString("ThemeColor.Text") % ";\
		}");
	}
	void MainWin::closeEvent(QCloseEvent* event) {
		Visindigo::Utility::JsonConfig* config = GlobalValue::getConfig();
		if (this->isMaximized()) {
			config->setBool("Window.Editor.Maximized", true);
		}
		else {
			config->setInt("Window.Editor.Width", this->width());
			config->setInt("Window.Editor.Height", this->height());
			config->setBool("Window.Editor.Maximized", false);
		}
		GlobalValue::saveConfig();
		GlobalValue::getCurrentProject()->saveProject();
	}
}