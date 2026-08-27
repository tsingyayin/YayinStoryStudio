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
#include "Editor/MainEditor/ToolWidgetArea.h"
#include "Editor/MainEditor/SimpleFileDialog.h"
#include "Editor/MainEditor/BottomInfoWidget.h"
#include "Editor/MainEditor/DebugServerRouter.h"
#include <Editor/DocumentMessageManager.h>
#include <Widgets/DesktopHacker.h>
#include <General/VIApplication.h>
#include <Editor/VirtualFilePath.h>
namespace YSS::Editor {
	MainWin* MainWin::Instance = nullptr;

	MainWin* MainWin::getInstance() {
		return Instance;
	}

	MainWin::MainWin() :QFrame() {
		Instance = this;
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon.png"));
		this->setWindowTitle("Yayin Story Studio");
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
		
		Browser = new ResourceBrowser(CentralWidget);
		Editors = new FileEditWidgetArea(CentralWidget);
		Tools = new ToolWidgetArea(CentralWidget);

		auto testSecondEditor = new FileEditWidgetArea();
		testSecondEditor->resize(800, 600);
		testSecondEditor->show();
		
		auto testThirdEditor = new FileEditWidgetArea();
		testThirdEditor->resize(800, 600);
		testThirdEditor->show();

		QObject::connect(Browser, &ResourceBrowser::visibilityChanged, Menu, &MainWinMenu::onResourceBrowserVisibilityChanged);

		Browser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
		splitter->addWidget(Browser);
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
		connect(Editors, &FileEditWidgetArea::renameRequested, this, [this](const QString& absOldPath) {
			RenameDlg->setContext(absOldPath);
			RenameDlg->show();
			});
		connect(Editors, &FileEditWidgetArea::saveAsRequested, this, [this](const QString& rawFilePath) {
			saveCurrentFocusedFileAs(rawFilePath);
			});
		connect(RenameDlg, &RenameDialog::renameConfirmed, this, [this](const QString& oldName, const QString& newName) {
			auto editor = YSSFSM->getFileEditWidget(oldName);
			if (editor) {
				editor->saveFile(newName, true);
			}
			Browser->refresh();
			});

		connect(Browser, &ResourceBrowser::fileRenamed, this, [this](const QString& path, const QString& oldName, const QString& newName) {
			const QString absOldPath = path + "/" + oldName;
			if (QFileInfo(absOldPath).isFile()) {
				auto editor = YSSFSM->getFileEditWidget(absOldPath);
				if (editor) {
					const QString absNewPath = path + "/" + newName;
					editor->saveFile(absNewPath, true);
				}
			}
			});
		connect(Browser, &ResourceBrowser::directoryRenamed, this, [this](const QString& path, const QString& oldName, const QString& newName) {
			const QString absOldPath = path + "/" + oldName;
			const QString absNewPath = path + "/" + newName;
			vgDebug << "Directory renamed from" << absOldPath << "to" << absNewPath;
			if (QFileInfo(absNewPath).isDir()) {
				auto newFileAbsPaths = Visindigo::Utility::FileUtility::fileFilter(absNewPath, {"*.*"}, true);
				vgDebug << "Renamed directory" << absOldPath << "to" << absNewPath << ", found" << newFileAbsPaths.size() << "files";
				for (auto newPath : newFileAbsPaths) {
					QString newRelativePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(absNewPath, newPath);
					newRelativePath = newRelativePath.mid(2);
					QString oldFileAbsPath = absOldPath + "/" + newRelativePath;
					auto editor = YSSFSM->getFileEditWidget(oldFileAbsPath);
					if (editor) {
						editor->saveFile(newPath, true);
					}
				}
			}
			});

		// NOTICE: Bad design here. temporary pointer should never be used in signal-slot connection.
		// If nobody handle the pointer, it will cause memory leak. And it`s ownership is not clear when
		// multiple slots are connected to the same signal.
		// As it`s not a public API, refactoring is not urgent.
		// SHOULD BE SOLVED in 0.17~0.18 version.
		connect(Browser, &ResourceBrowser::fileOperationRequested, this, [this](QUndoCommand* cmd) {
			cmd->redo();          // execute now
			// TODO: push to global QUndoStack when ready
			delete cmd;           // temporary — replace with stack management later
			Browser->refresh();
			});

		connect(Editors, &FileEditWidgetArea::currentFileChanged, this, [this](const QString& filePath) {
			BottomFrame->displayFileMessageCount(YSSCore::Editor::DocumentMessageManager::getInstance()->getMessageCount(filePath));
			auto currentEditWidget = Editors->getCurrentWidget();
			auto textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(currentEditWidget);
			if (textEdit) {
				BottomFrame->displayEditorInfo(textEdit->getTextCursor());
				BottomFrame->setEditorInfoEnable(true);
			}
			else {
				BottomFrame->setEditorInfoEnable(false);
			}
			if (not YSSCore::Editor::VirtualFilePath::isVirtualFilePath(filePath)) {
				Browser->setCurrentSelected(QFileInfo(filePath));
			}
		});

		connect(Editors, &FileEditWidgetArea::textEditCursorPositionChanged, this, [this](const QString& filePath, const QTextCursor& cursor) {
			BottomFrame->displayEditorInfo(cursor);
			});

		connect(BottomFrame, &BottomInfoWidget::requestStatistic, this, [this]() {
			auto currentEditWidget = Editors->getCurrentWidget();
			auto textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(currentEditWidget);
			QMessageBox box(this);
			box.setIcon(QMessageBox::NoIcon); // 不显示程序 Logo
			if (textEdit) {
				const QString fileName = QFileInfo(Editors->getCurrentWidgetFilePath()).fileName();
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
		Editors->addWidget(widget);
	}

	void MainWin::onToolWidgetOpened(const QString& widgetID) {
		Tools->addWidget(widgetID);
	}

	void MainWin::saveCurrentFocusedFile() {
		if (getFileEditWidgetArea()->getCurrentWidget()){
			getFileEditWidgetArea()->getCurrentWidget()->saveFile();
		}
	}

	void MainWin::saveCurrentFocusedFileAs(QString rawFilePath) {
		if (rawFilePath.isEmpty()) {
			rawFilePath = Editors->getCurrentWidget() ? Editors->getCurrentWidget()->getFilePath() : "";
		}
		if (rawFilePath.isEmpty()) {
			yDebugF << "No file to save as";
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
			Browser->refresh();
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

	void MainWin::openNewFileWindow() {
		Browser->openNewFileWindow();
	}

	void MainWin::help() {
		Visindigo::Utility::FileUtility::openBrowser("http://prts.site");
	}

	ResourceBrowser* MainWin::getResourceBrowser() {
		return Browser;
	}

	FileEditWidgetArea* MainWin::getFileEditWidgetArea() {
		return Editors;
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
		Editors->closeAll(true); // close all should be later than saveProject.
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
		YSSCore::General::YSSProject::getCurrentProject()->setFocusedFile(Editors->getCurrentWidget() ? Editors->getCurrentWidget()->getFilePath() : "");
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