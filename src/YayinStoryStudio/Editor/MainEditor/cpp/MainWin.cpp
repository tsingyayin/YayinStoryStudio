#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qmessagebox.h>
#include <Utility/JsonConfig.h>
#include <Widgets/ThemeManager.h>
#include <Widgets/QuickMenu.h>
#include <Utility/FileUtility.h>
#include <Editor/FileServerManager.h>
#include <General/YSSProject.h>
#include "../../ProjectPage/ProjectWin.h"
#include "../../GlobalValue.h"
#include "../MainWin.h"
#include "../FileEditorArea.h"
#include "../ResourceBrowser.h"
#include "../MenuBarHandler.h"
#include <General/YSSLogger.h>
namespace YSS::Editor {
	MainWin::MainWin() :QMainWindow() {
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		this->setWindowTitle("Yayin Story Studio");

		CentralWidget = new QWidget(this);
		this->setCentralWidget(CentralWidget);
		this->initMenu();
		this->setMinimumSize(800, 600);

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

		
		
		this->applyStyleSheet();
	}

	ResourceBrowser* MainWin::getResourceBrowser() {
		return Browser;
	}

	FileEditorArea* MainWin::getFileEditorArea() {
		return Editor;
	}

	void MainWin::backToProjectWin() {
		if (!checkProjectNeedToSave()) {
			return;
		}
		asked = true;
		this->close();
		saveProject();
		GlobalValue::setCurrentProject(nullptr);
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
		yDebugF << "Called";
		int width = GlobalValue::getConfig()->getInt("Window.Editor.Width");
		int height = GlobalValue::getConfig()->getInt("Window.Editor.Height");

		this->resize(width, height);
		if (GlobalValue::getConfig()->getBool("Window.Editor.Maximized")) {
			this->showMaximized();
		}
		GlobalValue::getCurrentProject()->refreshLastModifyTime();
		GlobalValue::getCurrentProject()->saveProject();
		QStringList openedFiles = GlobalValue::getCurrentProject()->getEditorOpenedFiles();
		for (const QString& filePath : openedFiles) {
			YSSFSM->openFile(filePath);
		}
		QString focusedFile = GlobalValue::getCurrentProject()->getFocusedFile();
		Editor->focusOn(focusedFile);
		this->CentralWidget->resize(this->width(), this->height() - Menu->height());
		yDebugF << CentralWidget->width() << CentralWidget->height();
		yDebugF << this->width() << this->height();
	}

	void MainWin::closeEvent(QCloseEvent* event) {
		yDebugF << "MainWin Close Event";
		if (!asked) {
			if (!checkProjectNeedToSave()) {
				event->ignore();
				return;
			}
			saveProject();
		}
		asked = false;
	}

	void MainWin::hideEvent(QHideEvent* event) {

	}

	void MainWin::resizeEvent(QResizeEvent* event) {
		QMainWindow::resizeEvent(event);
		this->CentralWidget->resize(this->width(), this->height() - Menu->height());
	}
	void MainWin::initMenu() {
		Menu = new Visindigo::Widgets::QuickMenu(this);
		Menu->setActionHandler(new YSS::Editor::MenuActionHandler(this));
		Menu->loadFromJson(Visindigo::Utility::FileUtility::readAll(":/yss/compiled/configWidget/mainEditorMenu.json"));
		this->setMenuBar(Menu);
		Menu->setStyleSheet("QMenuBar{border: 1px solid white}\
			QMenu{margin:10px;border: 1px solid " % YSSTM->getColorString("OutlineLight") % ";border-radius:5px;}\
			QMenu::item:selected{background-color: " % YSSTM->getColorString("Pressed") % "; color: " % YSSTM->getColorString("Text") % ";border-radius:5px;}\
			QMenuBar::item:selected{background-color: " % YSSTM->getColorString("Pressed") % "; color: " % YSSTM->getColorString("Text") % ";}\
			QMenu::separator{height:1px;background:" % YSSTM->getColorString("Pressed") % ";margin-left:5px;margin-right:5px;}\
");
	}

	bool MainWin::checkProjectNeedToSave() {
		YSSCore::General::YSSProject* project = GlobalValue::getCurrentProject();
		QMessageBox::StandardButton result = QMessageBox::question(this, "Save Project",
			"Do you want to save the project \"" % project->getProjectName() % "\" before closing?",
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
			QMessageBox::Yes);
		if (result == QMessageBox::Cancel) {
			return false;
		}
		else if (result == QMessageBox::Yes) {
			// sth to do , but not saveProject()
		}
		return true;
	}

	void MainWin::saveProject() {
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