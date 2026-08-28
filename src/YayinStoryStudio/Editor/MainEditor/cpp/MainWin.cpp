#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qfiledialog.h>
#include <General/YSSProject.h>
#include <General/YSSLogger.h>
#include <General/TranslationHost.h>
#include <General/PluginManager.h>
#include <General/Plugin.h>
#include <Editor/EditorPlugin.h>
#include <Editor/FileServerManager.h>
#include <Editor/TextEdit.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
#include <Utility/ColorTool.h>
#include <Utility/StringUtility.h>
#include <Widgets/ThemeManager.h>
#include "Editor/ProjectPage/ProjectWin.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/MainEditor/MainWinMenu.h"
#include "Editor/MainEditor/ResourceBrowser.h"
#include "Editor/MainEditor/FileOperationCommands.h"
#include "Editor/MainEditor/private/StackComponents_p.h"
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include "Editor/MainEditor/TreeLayoutWidget.h"
#include "Editor/MainEditor/ToolWidgetArea.h"
#include "Editor/MainEditor/SimpleFileDialog.h"
#include "Editor/MainEditor/BottomInfoWidget.h"
#include "Editor/MainEditor/DebugServerRouter.h"
#include <Editor/DocumentMessageManager.h>
#include <Widgets/DesktopHacker.h>
#include <General/VIApplication.h>
#include <Editor/VirtualFilePath.h>
#include "Editor/NewFilePage/NewFileWin.h"
#include <Editor/FileServer.h>
namespace YSS::Editor {
	MainWin* MainWin::Instance = nullptr;

	MainWin* MainWin::getInstance() {
		return Instance;
	}

	MainWin::MainWin() :QFrame() {
		Instance = this;
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon.png"));
		this->setWindowTitle(YSSCore::General::YSSProject::getCurrentProject()->getProjectName()+" - Yayin Story Studio");
		this->setMinimumSize(800, 600);

		MainLayout = new QVBoxLayout(this);
		MainLayout->setContentsMargins(0, 0, 0, 0);
		MainLayout->setSpacing(0);

		Menu = new MainWinMenu(this);
		this->setMenuShortcutTips();
		MainLayout->addWidget(Menu);

		CentralWidget = new QWidget(this);
		MainLayout->addWidget(CentralWidget);

		Layout = new QHBoxLayout(CentralWidget);
		Layout->setContentsMargins(10, 0, 10, 0);
		
		new ResourceBrowser(CentralWidget);
		auto Editors = new FileEditWidgetArea(CentralWidget);
		Tools = new ToolWidgetArea(CentralWidget);

		auto testTreeLayout = new TreeLayoutWidget();
		testTreeLayout->resize(800, 600);
		testTreeLayout->show();

		QObject::connect(ResourceBrowser::getInstance(), &ResourceBrowser::visibilityChanged, Menu, &MainWinMenu::onResourceBrowserVisibilityChanged);

		ResourceBrowser::getInstance()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		Editors->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		Tools->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

		QSplitter* hSplitter = new QSplitter(Qt::Vertical, CentralWidget);
		hSplitter->setContentsMargins(0, 0, 0, 0);
		hSplitter->addWidget(Editors);
		hSplitter->addWidget(Tools);
		hSplitter->setHandleWidth(8);
		hSplitter->setStretchFactor(0, 3);
		hSplitter->setStretchFactor(1, 1);
		QSplitter* splitter = new QSplitter(Qt::Horizontal, CentralWidget);
		splitter->setContentsMargins(0, 0, 0, 0);
		splitter->addWidget(ResourceBrowser::getInstance());
		splitter->addWidget(hSplitter);
		splitter->setHandleWidth(8);
		splitter->setStretchFactor(0, 1);
		splitter->setStretchFactor(1, 4);
		Layout->addWidget(splitter);

		BottomFrame = new BottomInfoWidget(this);
		BottomFrame->setFixedHeight(30);
		BottomFrame->setGitInfoEnable(false);
		MainLayout->addWidget(BottomFrame);

		connect(YSSDSR, &DebugServerRouter::actionStarted, this, [this](YSSCore::Editor::DebugServer::DebugAction action) {
			BottomFrame->displayDebugInfo(action, QString());
			});
		connect(YSSDSR, &DebugServerRouter::actionPercent, this, [this](YSSCore::Editor::DebugServer::DebugAction action, qint32 finished, qint32 total) {
			BottomFrame->displayDebugProgress(action, finished, total);
			});
		connect(YSSDSR, &DebugServerRouter::actionMessage, this, [this](YSSCore::Editor::DebugServer::DebugAction action, const QString& message) {
			BottomFrame->displayDebugInfo(action, message);
			});
		connect(YSSDSR, &DebugServerRouter::actionFinished, this, [this](YSSCore::Editor::DebugServer::DebugAction action, bool success) {
			BottomFrame->clearDebugInfo();
			BottomFrame->clearDebugProgress();
			});

		setColorfulEnable(true);
		onThemeChanged();

		qint64 width = VIApp->getMainPlugin()->getPluginConfig()->getInt("Window.Editor.Width");
		qint64 height = VIApp->getMainPlugin()->getPluginConfig()->getInt("Window.Editor.Height");

		this->resize(width, height);
		if (VIApp->getMainPlugin()->getPluginConfig()->getBool("Window.Editor.Maximized")) {
			this->showMaximized();
		}

		connect(YSSFSM, &YSSCore::Editor::FileServerManager::fileOpened, this, &MainWin::onFileEditOpened);
		connect(YSSTWM, &YSSCore::Editor::ToolWidgetManager::widgetOpened, this, &MainWin::onToolWidgetOpened);
		this->CentralWidget->resize(this->width(), this->height() - Menu->height());

		RenameDlg = new RenameDialog();
		RenameDlg->hide();
		connect(YSSCore::Editor::FileServerManager::getInstance(), &YSSCore::Editor::FileServerManager::focusOnFile,
			Editors, qOverload<const QString&, qint32, qint32>(&YSS::Editor::FileEditWidgetArea::setCurrentWidget));
		
		connect(RenameDlg, &RenameDialog::renameConfirmed, this, [this](const QString& oldName, const QString& newName) {
			auto editor = YSSFSM->getFileEditWidget(oldName);
			if (editor) {
				editor->saveFile(newName, true);
			}
			ResourceBrowser::getInstance()->refresh();
			});

		connect(BottomFrame, &BottomInfoWidget::requestStatistic, this, [this]() {
			auto currentEditWidget = FocusingFileEditWidgetNotTool;
			if (not FocusingFileEditWidgetNotTool) {
				return;
			}
			auto textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(currentEditWidget);
			QMessageBox box(this);
			box.setIcon(QMessageBox::NoIcon); // 不显示程序 Logo
			if (textEdit) {
				const QString fileName = QFileInfo(currentEditWidget->getFilePath()).fileName();
				box.setWindowTitle(VITR("YSS::editor.bottomInfoWidget.statistic.titleWithName").arg(fileName));
				auto stat = Visindigo::Utility::StringUtility::getStatistic(textEdit->getPlainText());
				QString message;
				message += VITR("YSS::editor.bottomInfoWidget.statistic.wordCount").arg(stat.WordCount) + "\n";
				message += VITR("YSS::editor.bottomInfoWidget.statistic.charCountIncludeWhitespace").arg(stat.CharCountIncludeWhitespace) + "\n";
				message += VITR("YSS::editor.bottomInfoWidget.statistic.charCountExcludeWhitespace").arg(stat.CharCountExcludeWhitespace) + "\n";
				message += VITR("YSS::editor.bottomInfoWidget.statistic.cjkvCharCount").arg(stat.CJKVCharCount) + "\n";
				message += VITR("YSS::editor.bottomInfoWidget.statistic.nonCJKVCharCount").arg(stat.NonCJKVCharCount) + "\n";
				message += VITR("YSS::editor.bottomInfoWidget.statistic.paragraphCount").arg(stat.ParagraphCount);
				box.setText(message);
			}
			else {
				box.setWindowTitle(VITR("YSS::editor.bottomInfoWidget.statistic.title"));
				box.setText(VITR("YSS::editor.bottomInfoWidget.statistic.noEditor"));
			}
			box.exec();
			});

		connect(YSSCore::Editor::DocumentMessageManager::getInstance(), &YSSCore::Editor::DocumentMessageManager::messageChanged, 
			this, [this](const QString& filePath) {
				BottomFrame->displayFileMessageCount(YSSCore::Editor::DocumentMessageManager::getInstance()->getMessageCount(filePath));
			});

		connect(YSSCore::Editor::DocumentMessageManager::getInstance(), &YSSCore::Editor::DocumentMessageManager::messageChangedForLine,
			this, [this](const QString& filePath, qint32 lineNumber) {
				BottomFrame->displayFileMessageCount(YSSCore::Editor::DocumentMessageManager::getInstance()->getMessageCount(filePath));
			});

		for (Visindigo::General::Plugin* plugin : VIPLM->getEnabledPlugins()) {
			if (plugin->getPluginExtensionID() == YSSPluginTypeID) {
				YSSCore::Editor::EditorPlugin* editorPlugin = dynamic_cast<YSSCore::Editor::EditorPlugin*>(plugin);
				if (editorPlugin) {
					editorPlugin->onProjectOpen(YSSCore::General::YSSProject::getCurrentProject());
				}
			}
		}

		YSSCore::General::YSSProject::getCurrentProject()->refreshLastModifyTime();
		YSSCore::General::YSSProject::getCurrentProject()->saveProject();
		QStringList openedFiles = YSSCore::General::YSSProject::getCurrentProject()->getEditorOpenedFiles();
		QString focusedFile = YSSCore::General::YSSProject::getCurrentProject()->getFocusedFile();
		QStringList stillOKFiles;
		for (const QString& filePath : openedFiles) {
			bool ok = YSSFSM->openFile(filePath);
			if (ok) {
				stillOKFiles.append(filePath);
			}
		}
		vgDebug << "Opened files" << stillOKFiles;
		YSSCore::General::YSSProject::getCurrentProject()->setEditorOpenedFiles(stillOKFiles);
		Editors->setCurrentWidget(focusedFile);
		YSSCore::General::YSSProject::getCurrentProject()->saveProject();

		YSSTWM->openToolWidget("cn.yxgeneral.yss.messageViewer");
	}

	MainWin::~MainWin() {
		RenameDlg->deleteLater();
		Instance = nullptr;
	}

	void MainWin::onFileEditOpened(const QString& filePath) {
		auto widget = YSSFSM->getFileEditWidget(filePath);
		for (auto area : FileEditWidgetArea::getAllAreas()) {
			if (area->containsWidget(filePath)) {
				area->addWidget(widget);
				return;
			}
		}
		lastFocusedFileEditArea->addWidget(widget);
	}

	void MainWin::onToolWidgetOpened(const QString& widgetID) {
		Tools->addWidget(widgetID);
	}

	void MainWin::saveCurrentFocusedFile() {
		if (FocusingFileEditWidgetNotTool) {
			FocusingFileEditWidgetNotTool->saveFile();
		}
		else if (FocusingFileEditWidget) {
			FocusingFileEditWidget->saveFile();
		}
		else {
			yDebugF << "No file to save";
		}
	}

	void MainWin::saveCurrentFocusedFileAs(QString rawFilePath) {
		if (rawFilePath.isEmpty()) {
			rawFilePath = FocusingFileEditWidgetNotTool ? FocusingFileEditWidgetNotTool->getFilePath() : "";
		}
		if (rawFilePath.isEmpty()) {
			yDebugF << "No file to save as";
			return;
		}
		if (YSSCore::Editor::VirtualFilePath::isVirtualFilePath(rawFilePath)) {
			yDebugF << "Cannot save virtual file as";
			return;
		}

		QString ext = QFileInfo(rawFilePath).suffix();
		QString newfilePath = QFileDialog::getSaveFileName(this,
			VITR("YSS::menu.file.saveAs"), rawFilePath, "(*." + ext + ")");
		if (newfilePath.isEmpty()) {
			return;
		}
		auto editor = YSSFSM->getFileEditWidget(newfilePath);
		if (editor) {
			if (editor->isFileChanged()) {
				QMessageBox::warning(this, VITR("YSS::editor.saveAsConflict.title"), VITR("YSS::editor.saveAsConflict.message").arg(newfilePath));
				return;
			}
			editor->closeFile();
		}
		editor = YSSFSM->getFileEditWidget(rawFilePath);
		if (editor) {
			editor->saveFile(newfilePath);
			ResourceBrowser::getInstance()->refresh();
		}
	}

	void MainWin::openFileDialog(const QString& startPath) {
		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		QDir CurrentDir;
		if (not startPath.isEmpty()) {
			CurrentDir.setPath(startPath);
		}
		else if (project) {
			CurrentDir.setPath(project->getProjectFolder());
		}
		else {
			CurrentDir.setPath(QDir::currentPath());
		}
		QString filePath = QFileDialog::getOpenFileName(
			nullptr,
			VITR("YSS::menu.file.open"),
			CurrentDir.absolutePath(),
			"All Files (*)");
		if (not filePath.isEmpty()) {
			YSSFSM->openFile(filePath);
		}
	}

	void MainWin::openNewFileWindow(const QString& startPath) {
		QString path = startPath;
		if (path.isEmpty()) {
			path = YSSCore::General::YSSProject::getCurrentProject()->getProjectFolder();
		}
		QFileInfo fileInfo(startPath);
		if (fileInfo.isFile()) {
			path = fileInfo.absoluteDir().absolutePath();
		}
		YSS::NewFilePage::NewFileWin* newFileWin = new YSS::NewFilePage::NewFileWin(path);
		newFileWin->setAttribute(Qt::WA_DeleteOnClose);
		newFileWin->setWindowModality(Qt::ApplicationModal);
		newFileWin->setWindowFlags(newFileWin->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
		connect(newFileWin, &YSS::NewFilePage::NewFileWin::filePrepared, this, [this](const QString& filePath) {
			if (QFileInfo(filePath).isFile()) {
				YSSFSM->openFile(filePath);
			}
			});
		newFileWin->show();
	}

	void MainWin::help() {
		Visindigo::Utility::FileUtility::openBrowser("http://prts.site");
	}

	void MainWin::saveAllFiles() {
		for (auto* widget : YSSFSM->getAllFileEditWidgets()) {
			if (widget->isFileChanged()) {
				widget->saveFile();
			}
		}
	}

	void MainWin::backToHome() {
		closeForBack = true;
		this->close();
	}

	void MainWin::setMenuShortcutTips() {
		static QMap<QString, QString> currentTips = {
			{"edit::undo", "Ctrl+Z"},
			{"edit::redo", "Ctrl+Y"},
			{"edit::cut", "Ctrl+X"},
			{"edit::copy", "Ctrl+C"},
			{"edit::paste", "Ctrl+V"},
			{"edit::selectAll", "Ctrl+A"},
			{"edit::findAndReplace", "Ctrl+F"},
		};
		Menu->setShortcutTips(currentTips);
	}

	FileEditWidgetArea* MainWin::getLastFocusedFileEditArea() const {
		return lastFocusedFileEditArea;
	}

	void MainWin::onFileEditWidgetAreaCreated(FileEditWidgetArea* area) {
		if (not lastFocusedFileEditArea) {
			lastFocusedFileEditArea = area;
		}
		connect(area, &FileEditWidgetArea::areaFocusd, this, &MainWin::onFileEditWidgetAreaFocusIn);
		connect(area, &FileEditWidgetArea::currentFileChanged, this, [this, area](const QString& filePath) {
			BottomFrame->displayFileMessageCount(YSSCore::Editor::DocumentMessageManager::getInstance()->getMessageCount(filePath));
			auto currentEditWidget = area->getCurrentWidget();
			if (not currentEditWidget) {
				FocusingFileEditWidget = nullptr;
				FocusingFileEditWidgetNotTool = nullptr;
				BottomFrame->setEditorInfoEnable(false);
				return;
			}
			bool isToolWidget = YSSFSM->getFileEditWidgetSourceServer(currentEditWidget)->isListAsTool();
			if (not isToolWidget) {
				FocusingFileEditWidgetNotTool = currentEditWidget;
			}
			FocusingFileEditWidget = currentEditWidget;
			auto textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(currentEditWidget);
			if (textEdit) {
				BottomFrame->displayEditorInfo(textEdit->getTextCursor());
				BottomFrame->setEditorInfoEnable(true);
			}
			else {
				BottomFrame->setEditorInfoEnable(false);
			}
			if (not YSSCore::Editor::VirtualFilePath::isVirtualFilePath(filePath)) {
				ResourceBrowser::getInstance()->setCurrentSelected(QFileInfo(filePath));
			}
			});
		connect(area, &FileEditWidgetArea::textEditCursorPositionChanged, this, [this](const QString& filePath, const QTextCursor& cursor) {
			BottomFrame->displayEditorInfo(cursor);
			});
		connect(area, &FileEditWidgetArea::renameRequested, this, [this](const QString& absOldPath) {
			RenameDlg->setContext(absOldPath);
			RenameDlg->show();
			});
		connect(area, &FileEditWidgetArea::saveAsRequested, this, [this](const QString& rawFilePath) {
			saveCurrentFocusedFileAs(rawFilePath);
			});
	}

	void MainWin::onFileEditWidgetAreaFocusIn(const QString& areaID) {
		if (not areaID.isEmpty()) {
			vgDebug << "FileEditWidgetArea focus in:" << areaID;
			auto area = FileEditWidgetArea::getAreaByID(areaID);
			if (area) {
				lastFocusedFileEditArea = area;
			}
		}
		if (lastFocusedFileEditArea){
			auto currentEditWidget = lastFocusedFileEditArea->getCurrentWidget();
			auto textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(currentEditWidget);
			if (textEdit) {
				BottomFrame->setEditorInfoEnable(true);
				BottomFrame->displayEditorInfo(textEdit->getTextCursor());
			}
			else {
				BottomFrame->setEditorInfoEnable(false);
			}
		}
		emit currentFileEditWidgetAreaChanged(lastFocusedFileEditArea);
	}

	void MainWin::onThemeChanged() {
		//this->applyVIStyleTemplate("YSS::MainWin");
	}

	void MainWin::showEvent(QShowEvent* event) {
		//yDebugF << CentralWidget->width() << CentralWidget->height();
		//yDebugF << this->width() << this->height();
		//Visindigo::Widgets::DesktopHacker::getInstance()->suspendQWidget(this);
	}

	void MainWin::closeEvent(QCloseEvent* event) {
		yDebugF << "MainWin Close Event";
		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		for (Visindigo::General::Plugin* plugin : VIPLM->getLoadedPlugins()) {
			if (plugin->getPluginExtensionID() == YSSPluginTypeID) {
				YSSCore::Editor::EditorPlugin* editorPlugin = dynamic_cast<YSSCore::Editor::EditorPlugin*>(plugin);
				if (editorPlugin) {
					bool okToClose = editorPlugin->onProjectAboutToClose(YSSCore::General::YSSProject::getCurrentProject());
					if (not okToClose) {
						event->ignore();
						closeForBack = false;
						return;
					}
				}
			}
		}

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
					editorPlugin->onProjectClose(YSSCore::General::YSSProject::getCurrentProject());
				}
			}
		}
		for (auto area : FileEditWidgetArea::getAllAreas()) {
			area->closeAll();
		}
		Tools->closeAll(); // this two lines indicates a potential memory trap. see comments in its destructor.
		Instance = nullptr;
		delete YSSCore::General::YSSProject::getCurrentProject();
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
		this->CentralWidget->resize(this->width(), this->height() - Menu->height() - BottomFrame->height());
	}

	void MainWin::saveProject() {
		saveAllFiles();
		YSSCore::General::YSSProject::getCurrentProject()->setFocusedFile(FocusingFileEditWidgetNotTool ? FocusingFileEditWidgetNotTool->getFilePath() : "");
		Visindigo::Utility::JsonConfig* config = VIApp->getMainPlugin()->getPluginConfig();
		if (this->isMaximized()) {
			config->setBool("Window.Editor.Maximized", true);
		}
		else {
			config->setInt("Window.Editor.Width", this->width());
			config->setInt("Window.Editor.Height", this->height());
			config->setBool("Window.Editor.Maximized", false);
		}
		VIApp->getMainPlugin()->savePluginConfig();
		YSSCore::General::YSSProject::getCurrentProject()->saveProject();
	}
}