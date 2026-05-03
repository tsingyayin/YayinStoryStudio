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
#include <Widgets/DesktopHacker.h>
#include "Editor/MainEditor/RenameDialog.h"
#include <QtWidgets/qfiledialog.h>

namespace YSS::Editor {
	MainWin* MainWin::Instance = nullptr;

	MainWin* MainWin::getInstance() {
		return Instance;
	}

	MainWin::MainWin() :QFrame() {
		Instance = this;
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/yssicon.png"));
		this->setWindowTitle("Yayin Story Studio");
		MainLayout = new QVBoxLayout(this);
		MainLayout->setContentsMargins(0, 0, 0, 0);
		MainLayout->setSpacing(0);
		this->initMenu();
		this->setMinimumSize(800, 600);
		//this->setWindowFlags(Qt::ExpandedClientAreaHint | Qt::NoTitleBarBackgroundHint);
		CentralWidget = new QWidget(this);
		MainLayout->addWidget(CentralWidget);
		Layout = new QHBoxLayout(CentralWidget);
		Layout->setContentsMargins(0, 0, 0, 0);
		QSplitter* splitter = new QSplitter(Qt::Horizontal, CentralWidget);
		splitter->setContentsMargins(0, 0, 0, 0);
		Editor = new StackWidgetArea(CentralWidget);
		Browser = new ResourceBrowser(CentralWidget);
		Browser->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
		Editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		Layout->addWidget(splitter);
		splitter->addWidget(Browser);
		splitter->addWidget(Editor);

		setColorfulEnable(true);
		onThemeChanged();
		
		for (Visindigo::General::Plugin* plugin : VIPLM->getLoadedPlugins()) {
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
		connect(YSSFSM, &YSSCore::Editor::FileServerManager::fileOpened, this, &MainWin::onFileEditOpened);
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

		RenameDlg = new RenameDialog();
		RenameDlg->hide();
		connect(Editor, &StackWidgetArea::renameRequested, this, [this](const QString& absOldPath) {
			RenameDlg->setContext(absOldPath);
			RenameDlg->show();
			});
		connect(Editor, &StackWidgetArea::saveAsRequested, this, [this](const QString& rawFilePath) {
			saveAs(rawFilePath);
			});
		connect(RenameDlg, &RenameDialog::renameConfirmed, this, [this](const QString& oldName, const QString& newName) {
			auto editor = YSSFSM->getFileEditWidget(oldName);
			if (editor) {
				editor->saveFile(newName, true);
			}
			Browser->refresh();
			});
	}

	MainWin::~MainWin() {
		RenameDlg->deleteLater();
	}

	void MainWin::onFileEditOpened(const QString& filePath) {
		auto widget = YSSFSM->getFileEditWidget(filePath);
		Editor->addWidget(widget);
	}

	void MainWin::saveAs(QString rawFilePath) {
		if (rawFilePath.isEmpty()) {
			rawFilePath = Editor->getCurrentWidget() ? Editor->getCurrentWidget()->getFilePath() : "";
		}
		if (rawFilePath.isEmpty()) {
			yDebugF << "No file to save as";
			return;
		}
		QString ext = QFileInfo(rawFilePath).suffix();
		QString newfilePath = QFileDialog::getSaveFileName(this,
			VITR("YSS::menu.file.saveAs"), rawFilePath, "(*."+ext+")");
		if (newfilePath.isEmpty()) {
			return;
		}
		auto editor = YSSFSM->getFileEditWidget(rawFilePath);
		if (editor) {
			editor->saveFile(newfilePath);
			Browser->refresh();
		}
	}

	void MainWin::openFile() {
		YSSCore::General::YSSProject* project = GlobalValue::getCurrentProject();
		QDir CurrentDir;
		if (project != nullptr) {
			CurrentDir.setPath(project->getProjectFolder());
		}
		else {
			CurrentDir.setPath(QDir::currentPath());
		}
		QString filePath = QFileDialog::getOpenFileName(
			nullptr,
			"Open File",
			CurrentDir.absolutePath(),
			"All Files (*)");
		if (!filePath.isEmpty()) {
			YSSFSM->openFile(filePath);
		}
	}

	void MainWin::help() {
		Visindigo::Utility::FileUtility::openBrowser("http://prts.site");
	}

	ResourceBrowser* MainWin::getResourceBrowser() {
		return Browser;
	}

	StackWidgetArea* MainWin::getStackWidgetArea() {
		return Editor;
	}

	void MainWin::saveAllFiles() {
		for (auto* widget : YSSFSM->getAllFileEditWidgets()) {
			widget->saveFile();
		}
	}

	void MainWin::backToProjectWin() {
		closeForBack = true;
		this->close();
	}

	void MainWin::onThemeChanged() {
		this->applyVIStyleTemplate("YSS::MainWin");
	}

	void MainWin::showEvent(QShowEvent* event) {
		//yDebugF << CentralWidget->width() << CentralWidget->height();
		//yDebugF << this->width() << this->height();
		//Visindigo::Widgets::DesktopHacker::getInstance()->suspendQWidget(this);
	}

	void MainWin::closeEvent(QCloseEvent* event) {
		yDebugF << "MainWin Close Event";
		YSSCore::General::YSSProject* project = GlobalValue::getCurrentProject();
		QMessageBox::StandardButton result = QMessageBox::question(this, VITR("YSS::project.saveQuestion.title"),
			VITR("YSS::project.saveQuestion.text").arg(project->getProjectName()),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
			QMessageBox::Yes);
		if (result == QMessageBox::Cancel) {
			event->ignore();
			closeForBack = false;
			return;
		}
		else if (result == QMessageBox::Yes) {
			saveProject();
		}

		for (Visindigo::General::Plugin* plugin : VIPLM->getLoadedPlugins()) {
			if (plugin->getPluginExtensionID() == YSSPluginTypeID) {
				YSSCore::Editor::EditorPlugin* editorPlugin = dynamic_cast<YSSCore::Editor::EditorPlugin*>(plugin);
				if (editorPlugin) {
					editorPlugin->onProjectClose(GlobalValue::getCurrentProject());
				}
			}
		}
		Editor->closeAll(); // close all should be later than saveProject.
		Instance = nullptr;
		delete GlobalValue::getCurrentProject();
		this->deleteLater();
		if (closeForBack) {
			YSS::ProjectPage::ProjectWin* win = new YSS::ProjectPage::ProjectWin();
			win->show();
		}
	}

	void MainWin::hideEvent(QHideEvent* event) {

	}

	void MainWin::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		this->CentralWidget->resize(this->width(), this->height() - Menu->height());
	}
	
	void MainWin::keyPressEvent(QKeyEvent* event) {
		QFrame::keyPressEvent(event);
		if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
			if (event->key() == Qt::Key_S) {
				saveProject();
			}
		}
		else if (event->modifiers() == Qt::ControlModifier) {
			if (event->key() == Qt::Key_S) {
				auto editor = Editor->getCurrentWidget();
				if (editor) {
					editor->saveFile();
				}
			}
			else if (event->key() == Qt::Key_O) {
				this->openFile();
			}
		}
		else if (event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
			if (event->key() == Qt::Key_S) {
				saveAs();
			}
		}
		else if(event->key() == Qt::Key_F1) {
			help();
		}
	}

	void MainWin::initMenu() {
		QHBoxLayout* topLayout = new QHBoxLayout();
		topLayout->setContentsMargins(10, 0, 10, 0);
		topLayout->setSpacing(0);
		Menu = new Visindigo::Widgets::QuickMenu(this);
		Menu->setActionHandler(new YSS::Editor::MenuActionHandler(this));
		Menu->loadFromJson(Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.yayinstorystudio/configWidget/mainEditorMenu.json"));
		Menu->setMaximumHeight(32);
		topLayout->addWidget(Menu);
		Visindigo::Widgets::BorderLabel* projectNameLabel = new Visindigo::Widgets::BorderLabel(this);
		projectNameLabel->setContentsMargins(10, 0, 10, 0);
		projectNameLabel->setText(GlobalValue::getCurrentProject()->getProjectName());
		projectNameLabel->setMaximumHeight(32);
		topLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		topLayout->addWidget(projectNameLabel);
		MainLayout->addLayout(topLayout);
	}

	void MainWin::saveProject() {
		saveAllFiles();
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