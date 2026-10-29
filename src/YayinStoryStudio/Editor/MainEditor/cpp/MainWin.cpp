#include <QtGui/qevent.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qtimer.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qsplitter.h>
#include <Editor/EditorPlugin.h>
#include <Editor/CodeDiff.h>
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
#include "Editor/MainEditor/TreeLayoutWidget.h"
#include "Editor/NewFilePage/NewFileWin.h"
#include "Editor/ProjectPage/ProjectWin.h"

VI_DECLARE_LOGGER(StartupProbe, YSSStartupProbe, YSS);

namespace YSS::Editor {
	MainWin* MainWin::Instance = nullptr;

	MainWin* MainWin::getInstance() {
		return Instance;
	}

	MainWin::MainWin() :QFrame() {
		StartupProbeTimer.start();
		YSSStartupProbe->debug() << "MainWin construction started";
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
		connect(this, &MainWin::currentFileEditWidgetChangedNotTool,
			MessageCenter::getInstance(), &MessageCenter::onCurrentFileEditWidgetChanged);
		connect(YSSFSM, &YSSCore::Editor::FileServerManager::fileOpened, this, &MainWin::onFileEditOpened);

		initTreeLayout();
		YSSStartupProbe->debug() << "initTreeLayout finished, elapsed(ms):" << StartupProbeTimer.elapsed();

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

		connect(YSSFSM, &YSSCore::Editor::FileServerManager::fileClosed, this, [this](const YSSCore::General::YSSURI& uri) {
			if (auto* current = FileEditWidgetArea::getCurrentFocusedWidget(); current && current->getUri() == uri) {
				FileEditWidgetArea::setCurrentFocusedWidget(FileEditWidgetArea::getCurrentArea(), nullptr);
			}
			if (FocusingFileEditWidgetNotTool && FocusingFileEditWidgetNotTool->getUri() == uri) {
				FocusingFileEditWidgetNotTool = nullptr;
			}
			});
		this->CentralWidget->resize(this->width(), this->height() - Menu->height());

		RenameDlg = new RenameDialog();
		RenameDlg->hide();
		
		connect(RenameDlg, &RenameDialog::renameConfirmed, this, [this](const QString& oldName, const QString& newName) {
			auto editor = YSSFSM->getFileEditWidget(oldName);
			if (editor) {
				editor->saveFile(YSSCore::General::YSSURI::fromLocalPath(newName), true);
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
					QElapsedTimer pluginTimer;
					pluginTimer.start();
					editorPlugin->onProjectOpen(YSSCore::General::YSSProject::getCurrentProject());
					YSSStartupProbe->debug() << "onProjectOpen finished, plugin:"
						<< plugin->getPluginID() << "elapsed(ms):" << pluginTimer.elapsed()
						<< "total(ms):" << StartupProbeTimer.elapsed();
				}
			}
		}
		YSSStartupProbe->debug() << "synchronous project initialization finished, elapsed(ms):"
			<< StartupProbeTimer.elapsed();

		YSSCore::General::YSSProject::getCurrentProject()->refreshLastModifyTime();
		YSSCore::General::YSSProject::getCurrentProject()->saveProject();
		YSSCore::General::YSSProject* currentProject = YSSCore::General::YSSProject::getCurrentProject();
		QMap<QString, QList<YSSCore::General::YSSURI>> openedFilesInArea = currentProject->getEditorOpenedFilesInArea();
		YSSCore::General::YSSURI focusedFile = currentProject->getFocusedFile();
		QMap<QString, QList<YSSCore::General::YSSURI>> stillOKFilesInArea;
		for (auto areaID : openedFilesInArea.keys()) {
			QList<YSSCore::General::YSSURI> stillOKFiles;
			for (const YSSCore::General::YSSURI& fileUri : openedFilesInArea[areaID]) {
				bool okToOpen = YSSFSM->openFile(fileUri);
				YSSStartupProbe->debug() << "restore file request:" << fileUri.toString()
					<< "accepted:" << okToOpen << "elapsed(ms):" << StartupProbeTimer.elapsed();
				if (okToOpen) {
					stillOKFiles.append(fileUri);
				}
			}
			if (not stillOKFiles.isEmpty()) {
				stillOKFilesInArea[areaID] = stillOKFiles;
			}
		}
		currentProject->setEditorOpenedFilesInArea(stillOKFilesInArea);
		// The focused file may live in a non-main area (e.g. a split window).
		// Always targeting "Editors" here would reparent that file's FileEditWidget
		// into the main area and leave the original area with a tag but no content.
		FileEditWidgetArea* focusedArea = FileEditWidgetArea::getAreaByID(currentProject->getFileAreaID(focusedFile));
		if (focusedArea and focusedFile.isValid()) {
			// 真实文件是异步读取的，此时控件可能还没建好；等 onFileEditOpened 再设为当前页签。
			YSSCore::Editor::FileEditWidget* focusedWidget = YSSFSM->getFileEditWidget(focusedFile);
			if (focusedWidget) {
				focusedArea->setCurrentWidget(focusedWidget);
			}
			FileEditWidgetArea::setCurrentFocusedWidget(focusedArea, focusedWidget);
		}
		saveProject();
		YSSStartupProbe->debug() << "MainWin construction finished, elapsed(ms):"
			<< StartupProbeTimer.elapsed();
	}

	MainWin::~MainWin() {
		// RenameDlg 没有父对象，deleteLater 依赖事件循环继续运行；进程退出路径上循环随时会停，
		// 所以这里同步销毁，保证它一定被释放。
		delete RenameDlg;
		RenameDlg = nullptr;
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

	static bool hasLayoutArea(const Visindigo::Utility::JsonConfig& layoutConfig) {
		for (const auto& child : layoutConfig.getArray("children")) {
			if (child.getString("type") == "layout") {
				if (hasLayoutArea(child)) {
					return true;
				}
			}
			else {
				return true;
			}
		}
		return false;
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
		const YSSCore::General::YSSURI fileUri = widget->getUri();
		const QString filePath = widget->getFilePath();
		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		// 项目中已记录了此文件所在的区域（例如上次退出时未关闭的文件），直接回到原区域。
		if (project) {
			QString recordedAreaID = project->getFileAreaID(fileUri);
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

	void MainWin::onFileEditOpened(const YSSCore::General::YSSURI& uri) {
		YSSStartupProbe->debug() << "file opened:" << uri.toString()
			<< "elapsed(ms):" << StartupProbeTimer.elapsed();
		auto widget = YSSFSM->getFileEditWidget(uri);
		if (not widget) {
			return;
		}

		if (auto sourceServer = YSSFSM->getFileEditWidgetSourceServer(widget)) {
			if (sourceServer->isListAsTool()) {
				Menu->syncPluginToolMenu();
			}
		}
		for (auto area : FileEditWidgetArea::getAllAreas()) {
			if (area->containsWidget(uri)) {
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
			editor->saveFile(YSSCore::General::YSSURI::fromLocalPath(newfilePath));
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
			CurrentDir.setPath(project->getProjectFolder().getLocalPath());
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

	void MainWin::openNewFileWindow(const YSSCore::General::YSSURI& startUri) {
		YSSCore::General::YSSURI folder = startUri;
		if (not folder.isValid()) {
			folder = YSSCore::General::YSSProject::getCurrentProject()->getProjectFolder();
		}
		else if (folder.isFile()) {
			// 选中了一个文件时，新文件建在它所在的目录里。
			folder = folder.parent();
		}
		YSS::NewFilePage::NewFileWin* newFileWin = new YSS::NewFilePage::NewFileWin(folder);
		newFileWin->setAttribute(Qt::WA_DeleteOnClose);
		newFileWin->setWindowModality(Qt::ApplicationModal);
		newFileWin->setWindowFlags(newFileWin->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
		connect(newFileWin, &YSS::NewFilePage::NewFileWin::filePrepared, this,
			[](const YSSCore::General::YSSURI& uri) {
				// 文件刚刚由新建窗口写好，直接交给文件服务管理器打开。
				YSSFSM->openFile(uri);
			});
		newFileWin->show();
	}

	void MainWin::help() {
		Visindigo::Utility::FileUtility::openBrowser("http://prts.site");
	}

	// 只负责提交保存请求，不等写盘结束；需要等结果时用 hasPendingSaves()/waitForPendingSaves()。
	void MainWin::saveAllFiles(bool silent) {
		for (auto* widget : YSSFSM->getAllFileEditWidgets()) {
			if (widget->isFileChanged()) {
				widget->saveFile(YSSCore::General::YSSURI(), false, silent);
			}
		}
	}

	void MainWin::backToHome() {
		CloseForBack = true;
		this->close();
	}

	bool MainWin::hasPendingSaves() const {
		for (const auto* widget : YSSFSM->getAllFileEditWidgets()) {
			if (widget->isSaving()) {
				return true;
			}
		}
		// 项目配置本身也是异步写盘的，关窗同样要等它写完。
		const YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		if (project != nullptr and project->isSaving()) {
			return true;
		}
		return false;
	}

	void MainWin::waitForPendingSaves() {
		if (CloseAwaitingSaves) {
			return;
		}
		CloseAwaitingSaves = true;
		CloseSaveFailed = false;
		CloseSaveFailedUri.clear();
		// 只关心本次等待期间提交的写入失败，所以用一个临时上下文对象挂这些连接，结束后整体断开。
		QObject* watcher = new QObject(this);
		for (auto* widget : YSSFSM->getAllFileEditWidgets()) {
			connect(widget, &YSSCore::Editor::FileEditWidget::fileSaveFailed, watcher,
				[this](const YSSCore::General::YSSURI& uri) {
					CloseSaveFailed = true;
					CloseSaveFailedUri = uri.toString();
					yWarningF << "Failed to save" << CloseSaveFailedUri << "while closing";
				});
		}
		if (YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject()) {
			connect(project, &YSSCore::General::YSSProject::projectSaveFailed, watcher, [this, project]() {
				CloseSaveFailed = true;
				CloseSaveFailedUri = project->getProjectPath().toString();
				yWarningF << "Failed to save project" << CloseSaveFailedUri << "while closing";
				});
		}
		QTimer* timer = new QTimer(this);
		timer->setInterval(50);
		connect(timer, &QTimer::timeout, this, [this, timer, watcher]() {
			if (CloseSaveFailed) {
				timer->stop();
				timer->deleteLater();
				delete watcher;
				CloseAwaitingSaves = false;
				CloseApproved = false;
				CloseForBack = false;
				QMessageBox::warning(this, VITRL("YSS::editor.fileOperation.failed.title"),
					VITRL("YSS::editor.fileOperation.failed.message").arg(CloseSaveFailedUri));
				return;
			}
			if (hasPendingSaves()) {
				return;
			}
			timer->stop();
			timer->deleteLater();
			delete watcher;
			CloseAwaitingSaves = false;
			CloseApproved = true;
			QMetaObject::invokeMethod(this, [this]() { this->close(); }, Qt::QueuedConnection);
		});
		timer->start();
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
		bool restored = false;
		if (YSSCore::General::YSSProject::getCurrentProject()->hasTreeLayoutConfig()) {
			auto treeConfig = YSSCore::General::YSSProject::getCurrentProject()->getTreeLayoutConfig();
			auto defaultConfig = treeConfig.getObject("default");
			if (hasLayoutArea(defaultConfig)) {
				TreeLayout->recoverFromJson(defaultConfig);
				for (auto key : treeConfig.keys("extra")) {
					auto extraConfig = treeConfig.getObject("extra." + key);
					if (not hasLayoutArea(extraConfig)) {
						continue;
					}
					auto treeLayout = new TreeLayoutWidget();
					treeLayout->recoverFromJson(extraConfig);
					treeLayout->show();
				}
				restored = true;
			}
		}
		if (not restored) {
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
		connect(area, &FileEditWidgetArea::currentFileChanged, this, [this, area](const YSSCore::General::YSSURI&) {
			// 后台区域换文件不算“当前文件”变了，只有当前聚焦的那个区域才跟。
			if (area != FileEditWidgetArea::getCurrentArea()) {
				return;
			}
			FileEditWidgetArea::setCurrentFocusedWidget(area, area->getCurrentWidget());
			});
		connect(area, &FileEditWidgetArea::textEditCursorPositionChanged, this, [this](const YSSCore::General::YSSURI& uri, const QTextCursor& cursor) {
			BottomFrame->displayEditorInfo(cursor);
			});
		connect(area, &FileEditWidgetArea::renameRequested, this, [this](const YSSCore::General::YSSURI& uri) {
			RenameDlg->setContext(uri.getLocalPath());
			RenameDlg->show();
			});
		connect(area, &FileEditWidgetArea::saveAsRequested, this, [this](const YSSCore::General::YSSURI& uri) {
			saveCurrentFocusedFileAs(uri.getLocalPath());
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
		YSSCore::Editor::FileServer* sourceServer = YSSFSM->getFileEditWidgetSourceServer(widget);
		if (sourceServer and sourceServer->isListAsTool()) {
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
		if (CloseAwaitingSaves) {
			// 正在等异步写入结束，期间的重复关闭请求一律忽略。
			event->ignore();
			return;
		}
		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		if (not CloseApproved) {
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
		}

		if (hasPendingSaves()) {
			// 项目和文件都是异步写盘的，必须等写完再销毁窗口，否则进程可能先退出。
			event->ignore();
			waitForPendingSaves();
			return;
		}

		// 用户已经确认关闭，此后不再需要为这个项目打开任何文件。还没读完的打开请求要在这里
		// 作废，否则它们读完后仍会创建编辑器，而那时已经没有区域能接管这些控件了。
		YSSFSM->clearPendingOpens();

		for (TreeLayoutWidget* layout : TreeLayoutWidget::getAllTopLevelLayouts()) {
			if (not layout->closeAllFilesWithConfirm()) {
				event->ignore();
				if (hasPendingSaves()) {
					// 逐个关闭时用户选择了保存：保留当前的关闭意图，等写完后再关一次。
					waitForPendingSaves();
				}
				else {
					CloseApproved = false;
					CloseForBack = false;
				}
				return;
			}
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
			if (not layout->close()) {
				event->ignore();
				CloseForBack = false;
				return;
			}
		}
		// 本窗口的编辑器都关掉了，而消息中心会活到下一个项目，必须让它忘掉这些编辑器。
		MessageCenter::getInstance()->onCurrentFileEditWidgetChanged(nullptr);
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

	void MainWin::saveProject(bool silent) {
		saveAllFiles(silent);
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

		YSSCore::General::YSSProject::getCurrentProject()->setFocusedFile(
			FocusingFileEditWidgetNotTool ? FocusingFileEditWidgetNotTool->getUri() : YSSCore::General::YSSURI());
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

	// 新旧两侧各自独立登记；另一侧已登记时立即打开比较器并清空两侧登记。
	void MainWin::setCompareFileAsOld(const YSSCore::General::YSSURI& uri) {
		if (!uri.isValid() || uri.isVirtual()) {
			return;
		}
		CompareOldUri = uri;
		HasCompareOld = true;
		launchComparisonIfReady();
	}

	void MainWin::setCompareFileAsNew(const YSSCore::General::YSSURI& uri) {
		if (!uri.isValid() || uri.isVirtual()) {
			return;
		}
		CompareNewUri = uri;
		HasCompareNew = true;
		launchComparisonIfReady();
	}

	void MainWin::launchComparisonIfReady() {
		if (!HasCompareOld || !HasCompareNew) {
			return;
		}
		const YSSCore::General::YSSURI oldUri = CompareOldUri;
		const YSSCore::General::YSSURI newUri = CompareNewUri;
		CompareOldUri = YSSCore::General::YSSURI();
		CompareNewUri = YSSCore::General::YSSURI();
		HasCompareOld = false;
		HasCompareNew = false;

		const QString fileName = QStringLiteral("%1 ↔ %2")
			.arg(oldUri.getFileName(), newUri.getFileName());
		Visindigo::Utility::JsonConfig params;
		params.setValue(QStringLiteral("old_uri"), QJsonValue(oldUri.toString()));
		params.setValue(QStringLiteral("new_uri"), QJsonValue(newUri.toString()));
		YSSCore::Editor::VirtualFilePath path(
			QStringLiteral(YSSVirtualFile_CodeDiff), fileName, params);
		if (!YSSFSM->openFile(path.toString())) {
			vgWarningF << "Failed to open code diff for" << oldUri.toString() << "and" << newUri.toString();
		}
	}
}