#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qsplitter.h>
#include <Editor/EditorPlugin.h>
#include <Editor/FileServer.h>
#include <Editor/FileServerManager.h>
#include <Editor/TextEdit.h>
#include <Editor/VirtualFilePath.h>
#include <General/Plugin.h>
#include <General/PluginManager.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <General/YSSLogger.h>
#include <General/YSSProject.h>
#include <Utility/ColorTool.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include <Utility/StringUtility.h>
#include <Widgets/DesktopHacker.h>
#include <Widgets/ThemeManager.h>
#include "Editor/MainEditor/BottomInfoWidget.h"
#include "Editor/MainEditor/DebugServerRouter.h"
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include "Editor/MainEditor/FileOperationCommands.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/MainEditor/MainWinMenu.h"
#include "Editor/MainEditor/MessageCenter.h"
#include "Editor/MainEditor/private/StackComponents_p.h"
#include "Editor/MainEditor/ResourceBrowser.h"
#include "Editor/MainEditor/SimpleFileDialog.h"
#include "Editor/MainEditor/ToolWidgetArea.h"
#include "Editor/MainEditor/TreeLayoutWidget.h"
#include "Editor/NewFilePage/NewFileWin.h"
#include "Editor/ProjectPage/ProjectWin.h"
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
		
		TreeLayout = new TreeLayoutWidget(CentralWidget);
		TreeLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		Layout->addWidget(TreeLayout);

		BottomFrame = new BottomInfoWidget(this);
		BottomFrame->setFixedHeight(30);
		BottomFrame->setGitInfoEnable(false);
		MainLayout->addWidget(BottomFrame);
		connect(MessageCenter::getInstance(), &MessageCenter::messageCountChanged, this, [this]() {
			BottomFrame->displayFileMessageCount(MessageCenter::getInstance()->getMessageCount());
			});
		connect(YSSFSM, &YSSCore::Editor::FileServerManager::fileOpened, this, &MainWin::onFileEditOpened);

		initTreeLayout();

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

		connect(YSSTWM, &YSSCore::Editor::ToolWidgetManager::widgetOpened, this, &MainWin::onToolWidgetOpened);
		connect(YSSFSM, &YSSCore::Editor::FileServerManager::fileClosed, this, [this](const QString& filePath) {
			if (auto* current = FileEditWidgetArea::getCurrentFocusedWidget(); current && current->getFilePath() == filePath) {
				FileEditWidgetArea::setCurrentFocusedWidget(FileEditWidgetArea::getCurrentArea(), nullptr);
			}
			if (FocusingFileEditWidgetNotTool && FocusingFileEditWidgetNotTool->getFilePath() == filePath) {
				FocusingFileEditWidgetNotTool = nullptr;
			}
			});
		this->CentralWidget->resize(this->width(), this->height() - Menu->height());

		RenameDlg = new RenameDialog();
		RenameDlg->hide();
		
		connect(RenameDlg, &RenameDialog::renameConfirmed, this, [this](const QString& oldName, const QString& newName) {
			auto editor = YSSFSM->getFileEditWidget(oldName);
			if (editor) {
				editor->saveFile(newName, true);
			}
			if (auto browser = ResourceBrowser::getInstance()) {
				browser->refresh();
			}
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
				box.setWindowTitle(VITRL("YSS::editor.bottomInfoWidget.statistic.titleWithName").arg(fileName));
				auto stat = Visindigo::Utility::StringUtility::getStatistic(textEdit->getPlainText());
				QString message;
				message += VITRL("YSS::editor.bottomInfoWidget.statistic.wordCount").arg(stat.WordCount) + "\n";
				message += VITRL("YSS::editor.bottomInfoWidget.statistic.charCountIncludeWhitespace").arg(stat.CharCountIncludeWhitespace) + "\n";
				message += VITRL("YSS::editor.bottomInfoWidget.statistic.charCountExcludeWhitespace").arg(stat.CharCountExcludeWhitespace) + "\n";
				message += VITRL("YSS::editor.bottomInfoWidget.statistic.cjkvCharCount").arg(stat.CJKVCharCount) + "\n";
				message += VITRL("YSS::editor.bottomInfoWidget.statistic.nonCJKVCharCount").arg(stat.NonCJKVCharCount) + "\n";
				message += VITRL("YSS::editor.bottomInfoWidget.statistic.paragraphCount").arg(stat.ParagraphCount);
				box.setText(message);
			}
			else {
				box.setWindowTitle(VITRL("YSS::editor.bottomInfoWidget.statistic.title"));
				box.setText(VITRL("YSS::editor.bottomInfoWidget.statistic.noEditor"));
			}
			box.exec();
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
		QMap<QString, QStringList> openedFilesInArea = YSSCore::General::YSSProject::getCurrentProject()->getEditorOpenedFilesInArea();
		QString focusedFile = YSSCore::General::YSSProject::getCurrentProject()->getFocusedFile();
		QMap<QString, QStringList> stillOKFilesInArea;
		for (auto areaID : openedFilesInArea.keys()) {
			QStringList stillOKFiles;
			for (auto filePath : openedFilesInArea[areaID]) {
				bool okToOpen = YSSFSM->openFile(filePath);
				if (okToOpen) {
					stillOKFiles.append(filePath);
				}
			}
			if (not stillOKFiles.isEmpty()) {
				stillOKFilesInArea[areaID] = stillOKFiles;
			}
		}
		YSSCore::General::YSSProject::getCurrentProject()->setEditorOpenedFilesInArea(stillOKFilesInArea);
		// The focused file may live in a non-main area (e.g. a split window).
		// Always targeting "Editors" here would reparent that file's FileEditWidget
		// into the main area and leave the original area with a tag but no content.
		FileEditWidgetArea* focusedArea = FileEditWidgetArea::getAreaByID(
			YSSCore::General::YSSProject::getCurrentProject()->getFileAreaID(focusedFile));
		if (focusedArea) {
			focusedArea->setCurrentWidget(focusedFile);
			FileEditWidgetArea::setCurrentFocusedWidget(focusedArea, YSSFSM->getFileEditWidget(focusedFile));
		}
		saveProject();
	}

	MainWin::~MainWin() {
		RenameDlg->deleteLater();
		Instance = nullptr;
	}

	// 在树形布局中查找直接包含 \a area 的布局，并把该区域在其中的下标写入 \a index。
	static TreeLayoutWidget* FindLayoutOfArea(TreeLayoutWidget* layout, FileEditWidgetArea* area, int& index) {
		for (int i = 0; i < layout->getChildCount(); i++) {
			if (layout->getFileEditAreaAt(i) == area) {
				index = i;
				return layout;
			}
			if (TreeLayoutWidget* childLayout = layout->getLayoutAt(i)) {
				if (TreeLayoutWidget* found = FindLayoutOfArea(childLayout, area, index)) {
					return found;
				}
			}
		}
		return nullptr;
	}

	// 在主区域的左侧或下侧就地创建一个新的副区域并返回它，创建失败时返回 nullptr。
	static FileEditWidgetArea* CreateSubAreaBesideMainArea() {
		FileEditWidgetArea* mainArea = FileEditWidgetArea::getMainArea();
		if (not mainArea) {
			return nullptr;
		}
		for (TreeLayoutWidget* topLevel : TreeLayoutWidget::getAllTopLevelLayouts()) {
			int index = -1;
			TreeLayoutWidget* host = FindLayoutOfArea(topLevel, mainArea, index);
			if (not host) {
				continue;
			}
			// 包裹主区域的新子布局方向与父布局相反：父布局为竖直方向时新布局为水平方向，
			// 新区域落在主区域左侧；父布局为水平方向时新区域落在主区域下侧。
			bool newFirst = (host->getOrientation() == Qt::Vertical);
			TreeLayoutWidget* newLayout = host->replaceFileEditAt(index, nullptr, newFirst);
			if (not newLayout) {
				continue;
			}
			return newLayout->getFileEditAreaAt(newFirst ? 0 : 1);
		}
		return nullptr;
	}

	YSS::Editor::FileEditWidgetArea* MainWin::selectFileEditWidgetAreaForNewFile(YSSCore::Editor::FileEditWidget* widget) {
		const QString filePath = widget->getFilePath();
		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		// 项目中已记录了此文件所在的区域（例如上次退出时未关闭的文件），直接回到原区域。
		if (project) {
			QString recordedAreaID = project->getFileAreaID(filePath);
			if (not recordedAreaID.isEmpty()) {
				if (FileEditWidgetArea* recordedArea = FileEditWidgetArea::getAreaByID(recordedAreaID)) {
					yDebug << "File opened in recorded area: " << recordedAreaID << " file: " << filePath;
					return recordedArea;
				}
			}
		}

		FileEditWidgetArea* mainArea = FileEditWidgetArea::getMainArea();
		yDebug << "Main area id" << (mainArea ? mainArea->getAreaID() : QString("none"));
		YSSCore::Editor::FileServer* sourceServer = YSSFSM->getFileEditWidgetSourceServer(widget);

		// 不作为工具的文件永远直接安排在主区域，且不考虑用户当前聚焦在哪个区域。
		if (not sourceServer or not sourceServer->isListAsTool()) {
			if (mainArea) {
				yDebug << "File opened as document, arrange in main area: " << filePath;
				return mainArea;
			}
		}
		else {
			// 作为工具的文件永远不安排在主区域，而是从副区域中选择一个与首选方向最匹配的区域。
			using PreferredOrientation = YSSCore::Editor::FileServer::PreferredOrientation;
			PreferredOrientation preferred = sourceServer->getPreferredOrientation();
			// 界面尚未真正显示时，各区域还没有可用的宽高，无法判断方向。此时一律按“任意区域”
			// 处理，避免程序启动、去恢复默认布局时误判长宽比而多创建一个副区域。
			bool geometryReady = this->isVisible();
			auto matchScore = [geometryReady, preferred](FileEditWidgetArea* area) -> int {
				// 分数越高越合适；-1 表示该区域不符合首选方向，不应选中。
				if (not geometryReady or area->width() <= 0 or area->height() <= 0) {
					return 0;
				}
				const double w = area->width();
				const double h = area->height();
				const bool tall = h > w;
				const bool wide = w > h;
				switch (preferred) {
				case PreferredOrientation::Any:
					return 0;
				case PreferredOrientation::Vertical:
					return tall ? 2 : -1;
				case PreferredOrientation::Vertical_Wide:
					if (not tall) {
						return -1;
					}
					return (h / w < 2.0) ? 2 : 1;
				case PreferredOrientation::Vertical_Narrow:
					if (not tall) {
						return -1;
					}
					return (h / w > 2.0) ? 2 : 1;
				case PreferredOrientation::Horizontal:
					return wide ? 2 : -1;
				case PreferredOrientation::Horizontal_Wide:
					if (not wide) {
						return -1;
					}
					return (w / h < 2.0) ? 2 : 1;
				case PreferredOrientation::Horizontal_Narrow:
					if (not wide) {
						return -1;
					}
					return (w / h > 2.0) ? 2 : 1;
				default:
					return 0;
				}
			};

			FileEditWidgetArea* best = nullptr;
			int bestScore = -1;
			for (FileEditWidgetArea* area : FileEditWidgetArea::getAllAreas()) {
				if (area == mainArea) {
					continue; // 工具永远不安排在主区域。
				}
				int score = matchScore(area);
				if (score < 0) {
					continue;
				}
				if (score > bestScore) {
					best = area;
					bestScore = score;
				}
				else if (score == bestScore and area == FileEditWidgetArea::getCurrentArea()) {
					// 同等条件下优先使用用户最后聚焦的那个副区域。
					best = area;
				}
			}
			if (best) {
				yDebug << "Tool file arranged in area: " << best->getAreaID() << " file: " << filePath;
				return best;
			}
			// 不存在合适的副区域，在主区域的左侧或下侧创建一个新的副区域来安排它。
			if (FileEditWidgetArea* newArea = CreateSubAreaBesideMainArea()) {
				yDebug << "Tool file arranged in new area: " << newArea->getAreaID() << " file: " << filePath;
				return newArea;
			}
		}

		// 兜底：最后聚焦的区域 -> 编号最小且不是主区域的区域 -> 主区域。
		yDebug << "Fallback area for file: " << filePath;
		if (FileEditWidgetArea* currentArea = FileEditWidgetArea::getCurrentArea()) {
			return currentArea;
		}
		for (FileEditWidgetArea* area : FileEditWidgetArea::getAllAreas()) {
			if (area != mainArea) {
				return area;
			}
		}
		return mainArea;
	}

	void MainWin::onFileEditOpened(const QString& filePath) {
		auto widget = YSSFSM->getFileEditWidget(filePath);
		if (not widget) {
			return;
		}

		if (auto sourceServer = YSSFSM->getFileEditWidgetSourceServer(widget)) {
			if (sourceServer->isListAsTool()) {
				Menu->syncPluginToolMenu();
			}
		}
		for (auto area : FileEditWidgetArea::getAllAreas()) {
			if (area->containsWidget(filePath)) {
				area->addWidget(widget);
				FileEditWidgetArea::setCurrentFocusedWidget(area, widget);
				return;
			}
		}
		if (auto area = selectFileEditWidgetAreaForNewFile(widget)) {
			area->addWidget(widget);
			FileEditWidgetArea::setCurrentFocusedWidget(area, widget);
		}
	}

	void MainWin::onToolWidgetOpened(const QString& widgetID) {
		if (Tools) {
			Tools->addWidget(widgetID);
		}
	}

	void MainWin::saveCurrentFocusedFile() {
		if (FocusingFileEditWidgetNotTool) {
			FocusingFileEditWidgetNotTool->saveFile();
		}
		else if (YSSCore::Editor::FileEditWidget* current = FileEditWidgetArea::getCurrentFocusedWidget()) {
			current->saveFile();
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
			VITRL("YSS::menu.file.saveAs"), rawFilePath, "(*." + ext + ")");
		if (newfilePath.isEmpty()) {
			return;
		}
		auto editor = YSSFSM->getFileEditWidget(newfilePath);
		if (editor) {
			if (editor->isFileChanged()) {
				QMessageBox::warning(this, VITRL("YSS::editor.saveAsConflict.title"), VITRL("YSS::editor.saveAsConflict.message").arg(newfilePath));
				return;
			}
			editor->closeFile();
		}
		editor = YSSFSM->getFileEditWidget(rawFilePath);
		if (editor) {
			editor->saveFile(newfilePath);
			if (auto browser = ResourceBrowser::getInstance()) {
				browser->refresh();
			}
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
			VITRL("YSS::menu.file.open"),
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
		CloseForBack = true;
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
		return FileEditWidgetArea::getCurrentArea();
	}

	YSSCore::Editor::FileEditWidget* MainWin::getCurrentFocusedFileEditWidget() const {
		return FileEditWidgetArea::getCurrentFocusedWidget();
	}

	YSSCore::Editor::FileEditWidget* MainWin::getCurrentFocusedFileEditWidgetNotTool() const {
		return FocusingFileEditWidgetNotTool;
	}

	void MainWin::initTreeLayout() {
		if (YSSCore::General::YSSProject::getCurrentProject()->hasTreeLayoutConfig()) {
			auto treeConfig = YSSCore::General::YSSProject::getCurrentProject()->getTreeLayoutConfig();
			TreeLayout->recoverFromJson(treeConfig.getObject("default"));
			for (auto key : treeConfig.keys("extra")) {
				auto treeLayout = new TreeLayoutWidget();
				treeLayout->recoverFromJson(treeConfig.getObject("extra." + key));
				treeLayout->show();
			}
		}
		else {
			TreeLayout->setOrientation(Qt::Horizontal);
			FileEditWidgetArea* resourceArea = TreeLayout->createFileEditAreaFirst();
			FileEditWidgetArea::setCurrentFocusedWidget(resourceArea, resourceArea ? resourceArea->getCurrentWidget() : nullptr);
			Menu->view_pluginTools(YSSFSM->getFileServerById("cn.yxgeneral.yss_builtin.resourceBrowserVFS"), true);
			FileEditWidgetArea* messageArea = new FileEditWidgetArea();
			TreeLayoutWidget* rightLayout = TreeLayout->replaceFileEditAt(1, messageArea, false);
			FileEditWidgetArea::setCurrentFocusedWidget(messageArea, messageArea->getCurrentWidget());
			Menu->view_pluginTools(YSSFSM->getFileServerById("cn.yxgeneral.yss_builtin.messageViewerVFS"), true);
			TreeLayout->setChildRatios({ 2, 5 });
			if (rightLayout) {
				rightLayout->setChildRatios({ 3, 1 });
			}
			FileEditWidgetArea* mainArea = FileEditWidgetArea::getMainArea();
			FileEditWidgetArea::setCurrentFocusedWidget(mainArea, mainArea ? mainArea->getCurrentWidget() : nullptr);
			if (mainArea) {
				mainArea->setFocus();
			}
		}
		// 确保主区域始终是编号最小的那个区域（恢复布局时创建顺序与编号顺序未必一致）。
		FileEditWidgetArea* smallestArea = nullptr;
		int smallestID = 0;
		for (FileEditWidgetArea* area : FileEditWidgetArea::getAllAreas()) {
			bool ok = false;
			int id = area->getAreaID().toInt(&ok);
			if (not ok) {
				continue;
			}
			if (not smallestArea or id < smallestID) {
				smallestArea = area;
				smallestID = id;
			}
		}
		if (smallestArea) {
			FileEditWidgetArea::changeMainArea(smallestArea);
		}
	}

	void MainWin::onFileEditWidgetAreaCreated(FileEditWidgetArea* area) {
		connect(area, &FileEditWidgetArea::areaFocusd, this, &MainWin::onFileEditWidgetAreaFocusIn);
		connect(area, &FileEditWidgetArea::currentFileChanged, this, [this, area](const QString&) {
			// 后台区域换文件不算“当前文件”变了，只有当前聚焦的那个区域才跟。
			if (area != FileEditWidgetArea::getCurrentArea()) {
				return;
			}
			FileEditWidgetArea::setCurrentFocusedWidget(area, area->getCurrentWidget());
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
		connect(area, &FileEditWidgetArea::areaClosed, this, [this, area](const QString& areaID) {
			if (YSSCore::General::YSSProject::getCurrentProject()) {
				YSSCore::General::YSSProject::getCurrentProject()->removeEditorOpenedFilesInArea(areaID);
			}
			});
	}

	void MainWin::onGlobalCurrentChanged(FileEditWidgetArea* area, YSSCore::Editor::FileEditWidget* widget) {
		emit currentFileEditWidgetChanged(widget);
		emit currentFileEditWidgetAreaChanged(area);
		if (widget == nullptr) {
			FocusingFileEditWidgetNotTool = nullptr;
			BottomFrame->setEditorInfoEnable(false);
			emit currentFileEditWidgetChangedNotTool(nullptr);
			return;
		}
		// 当前文件要在资源浏览器里跟着选中（虚拟文件不在资源浏览器里，跳过）。
		const QString filePath = widget->getFilePath();
		if (not filePath.isEmpty() and not YSSCore::Editor::VirtualFilePath::isVirtualFilePath(filePath)) {
			if (auto browser = ResourceBrowser::getInstance()) {
				browser->setCurrentSelected(QFileInfo(filePath));
			}
		}
		if (YSSFSM->getFileEditWidgetSourceServer(widget)->isListAsTool()) {
			return;
		}
		FocusingFileEditWidgetNotTool = widget;
		auto textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(widget);
		if (textEdit) {
			BottomFrame->displayEditorInfo(textEdit->getTextCursor());
			BottomFrame->setEditorInfoEnable(true);
		}
		else {
			BottomFrame->setEditorInfoEnable(false);
		}
		emit currentFileEditWidgetChangedNotTool(widget);
	}

	void MainWin::onFileEditWidgetAreaFocusIn(const QString& areaID) {
		if (areaID.isEmpty()) {
			return;
		}
		vgDebug << "FileEditWidgetArea focus in:" << areaID;
		FileEditWidgetArea* area = FileEditWidgetArea::getAreaByID(areaID);
		FileEditWidgetArea::setCurrentFocusedWidget(area, area ? area->getCurrentWidget() : nullptr);
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
						CloseForBack = false;
						return;
					}
				}
			}
		}

		QMessageBox::StandardButton result = QMessageBox::question(this, VITRL("YSS::project.saveQuestion.title"),
			VITRL("YSS::project.saveQuestion.text").arg(project->getProjectName()),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
			QMessageBox::Yes);
		if (result == QMessageBox::Cancel) {
			event->ignore();
			CloseForBack = false;
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
		for (auto layout : TreeLayoutWidget::getAllTopLevelLayouts()) {
			layout->close();
		}
		if (Tools) {
			Tools->closeAll(); // this two lines indicates a potential memory trap. see comments in its destructor.
		}
		Instance = nullptr;
		delete YSSCore::General::YSSProject::getCurrentProject();
		this->deleteLater();
		if (CloseForBack) {
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
		FileEditWidgetArea::compressAreaID();
		auto layouts = TreeLayoutWidget::getAllTopLevelLayouts();
		Visindigo::Utility::JsonConfig treeConfig;
		treeConfig.setObject("default", TreeLayout->saveToJson());
		for (auto layout : layouts) {
			if (layout != TreeLayout) {
				treeConfig.setObject("extra.s_" + layout->getTopLayoutID(), layout->saveToJson());
			}
		}
		YSSCore::General::YSSProject::getCurrentProject()->setTreeLayoutConfig(treeConfig);

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