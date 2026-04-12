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
#include "../ResourceBrowser.h"
#include "../MenuBarHandler.h"
#include <General/YSSLogger.h>
#include <Utility/ColorTool.h>
#include <General/TranslationHost.h>
#include <General/PluginManager.h>
#include <General/Plugin.h>
#include <Editor/EditorPlugin.h>
#include "Editor/MainEditor/StackWidgetArea.h"
namespace YSS::Editor {
	MainWin* MainWin::Instance = nullptr;

	MainWin* MainWin::getInstance() {
		return Instance;
	}

	MainWin::MainWin() :QMainWindow() {
		Instance = this;
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/yssicon.png"));
		this->setWindowTitle("Yayin Story Studio");

		CentralWidget = new QWidget(this);
		this->setCentralWidget(CentralWidget);
		this->initMenu();
		this->setMinimumSize(800, 600);

		Layout = new QHBoxLayout(CentralWidget);
		QSplitter* splitter = new QSplitter(Qt::Horizontal, CentralWidget);
		Editor = new StackWidgetArea(CentralWidget);
		Browser = new ResourceBrowser(CentralWidget);
		Browser->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
		Editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		Layout->addWidget(splitter);
		splitter->addWidget(Browser);
		splitter->addWidget(Editor);

		connect(YSSFSM, &YSSCore::Editor::FileServerManager::builtinEditorCreated,
			Editor, &StackWidgetArea::addWidget);
		connect(YSSFSM, &YSSCore::Editor::FileServerManager::switchLineEdit, Editor,
			qOverload<const QString&, qint32, qint32>(&StackWidgetArea::setCurrentWidget));
		
		setColorfulEnable(true);
		onThemeChanged();
		for (Visindigo::General::Plugin* plugin: VIPLM->getLoadedPlugins()) {
			if (plugin->getPluginExtensionID() == YSSPluginTypeID) {
				YSSCore::Editor::EditorPlugin* editorPlugin = dynamic_cast<YSSCore::Editor::EditorPlugin*>(plugin);
				if (editorPlugin) {
					editorPlugin->onProjectOpen(GlobalValue::getCurrentProject());
				}
			}
		}
		int width = GlobalValue::getConfig()->getInt("Window.Editor.Width");
		int height = GlobalValue::getConfig()->getInt("Window.Editor.Height");

		this->resize(width, height);
		if (GlobalValue::getConfig()->getBool("Window.Editor.Maximized")) {
			this->showMaximized();
		}
		GlobalValue::getCurrentProject()->refreshLastModifyTime();
		GlobalValue::getCurrentProject()->saveProject();
		QStringList openedFiles = GlobalValue::getCurrentProject()->getEditorOpenedFiles();
		QString focusedFile = GlobalValue::getCurrentProject()->getFocusedFile();
		QStringList stillOKFiles;
		for (const QString& filePath : openedFiles) {
			bool ok  = YSSFSM->openFile(filePath);
			if (ok) {
				stillOKFiles.append(filePath);
			}
		}
		GlobalValue::getCurrentProject()->setEditorOpenedFiles(stillOKFiles);
		Editor->setCurrentWidget(focusedFile);
		this->CentralWidget->resize(this->width(), this->height() - Menu->height());
	}

	ResourceBrowser* MainWin::getResourceBrowser() {
		return Browser;
	}

	StackWidgetArea* MainWin::getStackWidgetArea() {
		return Editor;
	}

	void MainWin::saveAll() {
		for (YSSCore::Editor::FileEditWidget* widget : Editor->getAllWidgets()) {
			widget->saveFile();
		}
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

	void MainWin::onThemeChanged() {
		this->applyVIStyleTemplate("YSS::MainWin");
	}

	void MainWin::showEvent(QShowEvent* event) {
		//yDebugF << CentralWidget->width() << CentralWidget->height();
		//yDebugF << this->width() << this->height();
	}

	void MainWin::closeEvent(QCloseEvent* event) {
		yDebugF << "MainWin Close Event";
		if (!asked) {
			if (!checkProjectNeedToSave()) {
				event->ignore();
				return;
			}
			for (Visindigo::General::Plugin* plugin : VIPLM->getLoadedPlugins()) {
				
				if (plugin->getPluginExtensionID() == YSSPluginTypeID) {
					YSSCore::Editor::EditorPlugin* editorPlugin = dynamic_cast<YSSCore::Editor::EditorPlugin*>(plugin);
					if (editorPlugin) {
						editorPlugin->onProjectClose(GlobalValue::getCurrentProject());
					}
				}
			}
			saveProject();
		}
		asked = false;
		Instance = nullptr;
		this->deleteLater();
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
		Menu->loadFromJson(Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.yayinstorystudio/configWidget/mainEditorMenu.json"));
		this->setMenuBar(Menu);
	}

	bool MainWin::checkProjectNeedToSave() {
		YSSCore::General::YSSProject* project = GlobalValue::getCurrentProject();
		QMessageBox::StandardButton result = QMessageBox::question(this, VITR("YSS::project.saveQuestion.title"),
			VITR("YSS::project.saveQuestion.text").arg(project->getProjectName()),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
			QMessageBox::Yes);
		if (result == QMessageBox::Cancel) {
			return false;
		}
		else if (result == QMessageBox::Yes) {
			saveAll();
		}
		return true;
	}

	void MainWin::saveProject() {
		GlobalValue::getCurrentProject()->setFocusedFile(Editor->getCurrentWidget() ? Editor->getCurrentWidget()->getFilePath() : "");
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