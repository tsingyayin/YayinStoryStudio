#include "../MainWin.h"
#include "../FileEditorArea.h"
#include "../ResourceBrowser.h"
#include "../../GlobalValue.h"
#include <Utility/JsonConfig.h>
#include <Editor/ThemeManager.h>
#include <Editor/FileServerManager.h>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QSplitter>

namespace YSS::Editor {

	MainWin::MainWin() :QMainWindow() {
		this->setWindowIcon(QIcon(":/compiled/yssicon.png"));
		int width = GlobalValue::getConfig()->getInt("Editor.Window.Width");
		int height = GlobalValue::getConfig()->getInt("Editor.Window.Height");
		this->setMinimumSize(800, 600);
		this->resize(width, height);
		this->setWindowTitle("Yayin Story Studio");
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
		
		connect(YSSFSM, &YSSCore::Editor::FileServerManager::builtinEditorCreated,
			Editor, &FileEditorArea::addFileEditWidget);

		this->setCentralWidget(CentralWidget);
		this->initMenu();
		this->applyStyleSheet();
	}

	void MainWin::initMenu() {
		MenuBar = new QMenuBar(this);
		this->setMenuBar(MenuBar);
		FileMenu = MenuBar->addMenu(GlobalValue::getTr("General.File"));
		SaveAction = new QAction(GlobalValue::getTr("General.Save"), this);
		SaveAsAction = new QAction(GlobalValue::getTr("General.SaveAs"), this);
		FileMenu->addAction(SaveAction);
		FileMenu->addAction(SaveAsAction);
	}

	void MainWin::applyStyleSheet() {
		this->setStyleSheet("QWidget{\
			background-color: "%YSSTM->getColorString("ThemeColor.Background") % ";\
			color: "%YSSTM->getColorString("ThemeColor.Text") % ";\
		}");
	}
	void MainWin::closeEvent(QCloseEvent* event) {
		YSSCore::Utility::JsonConfig* config = GlobalValue::getConfig();
		config->getInt("Editor.Window.Width");
		config->setInt("Editor.Window.Width", this->width());
		config->setInt("Editor.Window.Height", this->height());
		GlobalValue::saveConfig();
	}
}