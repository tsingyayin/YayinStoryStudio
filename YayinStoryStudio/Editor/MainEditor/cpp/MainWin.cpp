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
#include <QSplitter>
#include <General/TranslationHost.h>
#include <General/YSSProject.h>
#include "../MenuBarHandler.h"
#include "../../ProjectPage/ProjectWin.h"

namespace YSS::Editor {
	MainWin::MainWin() :QMainWindow() {
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		this->setMinimumSize(800, 600);
		this->setWindowTitle("Yayin Story Studio");

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
		
	}

	ResourceBrowser* MainWin::getResourceBrowser() {
		return Browser;
	}

	FileEditorArea* MainWin::getFileEditorArea() {
		return Editor;
	}

	void MainWin::backToProjectWin() {
		this->hide();
		if (GlobalValue::getCurrentProject()) {
			GlobalValue::setCurrentProject(nullptr);
		}
		YSS::ProjectPage::ProjectWin* win = new YSS::ProjectPage::ProjectWin();
		win->show();
	}

	void MainWin::applyStyleSheet() {
		this->setStyleSheet("QWidget{\
			background-color: " % YSSTM->getColorString("Background") % ";\
			color: " % YSSTM->getColorString("Text") % ";\
		}");
	}

	void MainWin::showEvent(QShowEvent* event) {
		int width = GlobalValue::getConfig()->getInt("Window.Editor.Width");
		int height = GlobalValue::getConfig()->getInt("Window.Editor.Height");
		
		if (GlobalValue::getConfig()->getBool("Window.Editor.Maximized")) {
			this->showMaximized();
		}
		else {
			this->resize(width, height);
		}
		GlobalValue::getCurrentProject()->refreshLastModifyTime();
		GlobalValue::getCurrentProject()->saveProject();
		QStringList openedFiles = GlobalValue::getCurrentProject()->getEditorOpenedFiles();
		for (const QString& filePath : openedFiles) {
			YSSFSM->openFile(filePath);
		}
		QString focusedFile = GlobalValue::getCurrentProject()->getFocusedFile();
		Editor->focusOn(focusedFile);
		this->repaint();
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

	void MainWin::hideEvent(QHideEvent* event) {
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
	void MainWin::initMenu() {
		Menu = new Visindigo::Widgets::QuickMenu(this);
		Menu->setActionHandler(new YSS::Editor::MenuActionHandler(this));
		Menu->loadFromJson(Visindigo::Utility::FileUtility::readAll(":/yss/compiled/configWidget/mainEditorMenu.json"));
		this->setMenuBar(Menu);
		Menu->setStyleSheet("QMenuBar{border: 1px solid white}\
			QMenu{margin:10px;border: 1px solid "%YSSTM->getColorString("OutlineLight") % ";border-radius:5px;}\
			QMenu::item:selected{background-color: " % YSSTM->getColorString("Pressed") % "; color: " % YSSTM->getColorString("Text") % ";border-radius:5px;}\
			QMenuBar::item:selected{background-color: " % YSSTM->getColorString("Highlight") % "; color: " % YSSTM->getColorString("HighlightedText") % ";}\
			QMenu::separator{height:1px;background:" % YSSTM->getColorString("Separator") % ";margin-left:5px;margin-right:5px;}\
");
	}

	
	
}