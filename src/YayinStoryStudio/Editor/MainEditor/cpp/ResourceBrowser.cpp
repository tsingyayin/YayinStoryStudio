#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qfilesystemmodel.h>
#include <QtWidgets/qapplication.h>
#include <QtCore/qmimedata.h>
#include <QtCore/qurl.h>
#include <QtCore/qdatastream.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qtoolbar.h>
#include <QtGui/qpainter.h>
#include <QtGui/qclipboard.h>
#include <General/TranslationHost.h>
#include <General/Log.h>
#include <General/VIApplication.h>
#include <General/YSSProject.h>
#include <Editor/FileServerManager.h>
#include <Editor/FileTemplateManager.h>
#include "Editor/MainEditor/ResourceBrowser.h"
#include <Widgets/ThemeManager.h>
#include "Editor/MainEditor/SimpleFileDialog.h"
#include "Editor/MainEditor/FileOperationCommands.h"
#include "Editor/MainEditor/MainWin.h"
#include <QtWidgets/qmessagebox.h>
#include <Utility/FileUtility.h>
namespace YSS::Editor {
	static ResourceBrowser* Instance = nullptr;
	ResourceBrowserVFS::ResourceBrowserVFS(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("YSS Built-in Resource Browser", "cn.yxgeneral.yss_builtin.resourceBrowserVFS", plugin) {
		setEditorType(YSSCore::Editor::FileServer::BuiltInEditor);
		setSupportedFileExts({ "YSS.MainEditor.ResourceBrowser" });
		setAsVitrualFileServer(true);
		setPreferredOrientation(YSSCore::Editor::FileServer::Vertical_Narrow);
		setListAsTool(true);
		setToolNickname("i18n:YSS::menu.view.resourceBrowser");
	}

	YSSCore::Editor::FileEditWidget* ResourceBrowserVFS::onCreateFileEditWidget() {
		return new ResourceBrowser();
	}

	ResourceBrowser::ResourceBrowser(QWidget* parent) :YSSCore::Editor::FileEditWidget(parent) {
		Instance = this;

		Layout = new QVBoxLayout(this);
		Layout->setSpacing(0);
		Layout->setContentsMargins(0, 0, 0, 0);

		ToolBar = new QToolBar(this);
		ToolActionRefresh = ToolBar->addAction(VITR("Visindigo::general.refresh"));
		ToolActionShrink = ToolBar->addAction(VITR("Visindigo::general.shrink"));
		ToolActionExpand = ToolBar->addAction(VITR("Visindigo::general.expand"));
		ToolActionNewFile = ToolBar->addAction(VITR("Visindigo::general.new"));
		ToolActionNewFolder = ToolBar->addAction(VITR("YSS::menu.file.newFolder"));
		ToolBar->setMaximumHeight(40);
		Layout->addWidget(ToolBar);

		connect(ToolActionRefresh, &QAction::triggered, this, &ResourceBrowser::refreshFileList);
		connect(ToolActionNewFile, &QAction::triggered, this, &ResourceBrowser::onNewButtonClicked);
		connect(ToolActionNewFolder, &QAction::triggered, this, &ResourceBrowser::onNewFolderTriggered);
		connect(ToolActionShrink, &QAction::triggered, this, [this]() {
			FileTree->collapseAll();
			});
		connect(ToolActionExpand, &QAction::triggered, this, [this]() {
			FileTree->expandAll();
			});

		FileTree = new QTreeView(this);
		FileTree->setContextMenuPolicy(Qt::CustomContextMenu);
		FileTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
		Layout->addWidget(FileTree);
		
		FileModel = new QFileSystemModel(this);
		FileModel->setReadOnly(false);
		FileTree->setModel(FileModel);
		FileTree->setHeaderHidden(true);
		
		connect(FileTree, &QTreeView::doubleClicked, this, &ResourceBrowser::onItemDoubleClicked);
		connect(FileTree, &QTreeView::customContextMenuRequested, this, &ResourceBrowser::onFileTreeContextMenuRequested);

		FileOptions = new QMenu(this);
		FileOptionOpen = FileOptions->addAction(VITR("Visindigo::general.open"));
		FileOptionRename = FileOptions->addAction(VITR("Visindigo::general.rename"));
		FileOptionDelete = FileOptions->addAction(VITR("Visindigo::general.delete"));
		FileOptionNewFile = FileOptions->addAction(VITR("YSS::menu.file.newFile"));
		FileOptionNewFolder = FileOptions->addAction(VITR("YSS::menu.file.newFolder"));
		FileOptions->addSeparator();
		FileOptionCopyPath = FileOptions->addAction(VITR("YSS::menu.edit.copyPath"));
		FileOptionCopyName = FileOptions->addAction(VITR("YSS::menu.edit.copyName"));
		FileOptions->addSeparator();
		FileOptionCopy = FileOptions->addAction(VITR("YSS::menu.edit.copy"));
		FileOptionPaste = FileOptions->addAction(VITR("YSS::menu.edit.paste"));
		FileOptionCut = FileOptions->addAction(VITR("YSS::menu.edit.cut"));
		FileOptions->addSeparator();
		FileOptionRefresh = FileOptions->addAction(VITR("Visindigo::general.refresh"));
		
		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		if (project != nullptr) {
			ProjectRootDir.setPath(project->getProjectFolder());
			FileTree->setRootIndex(FileModel->setRootPath(project->getProjectFolder()));
			for (int i = 1; i < FileModel->columnCount(); i++) {
				FileTree->setColumnHidden(i, true);
			}
		}
		else {
			ProjectRootDir.setPath(QDir::currentPath());
			FileTree->setRootIndex(FileModel->setRootPath(QDir::currentPath()));
			for (int i = 1; i < FileModel->columnCount(); i++) {
				FileTree->setColumnHidden(i, true);
			}
		}
		

		connect(FileOptionOpen, &QAction::triggered, this, [this]() {
			auto fileInfo = QFileInfo(CurrentFilePath);
			if (fileInfo.isFile()) {
				YSSFSM->openFile(CurrentFilePath);
			}
			else if (fileInfo.isDir()) {
				FileTree->expand(RightClickedIndex);
			}
			});
		connect(FileOptionNewFile, &QAction::triggered, this, &ResourceBrowser::onNewButtonClicked);
		connect(FileOptionNewFolder, &QAction::triggered, this, &ResourceBrowser::onNewFolderTriggered);
		connect(FileOptionRename, &QAction::triggered, this, &ResourceBrowser::onRenameTriggered);
		connect(FileOptionRefresh, &QAction::triggered, this, &ResourceBrowser::refreshFileList);
		connect(FileOptionCopyPath, &QAction::triggered, this, [this]() {
			QApplication::clipboard()->setText(CurrentFilePath);
			});
		connect(FileOptionCopyName, &QAction::triggered, this, [this]() {
			QApplication::clipboard()->setText(QFileInfo(CurrentFilePath).fileName());
			});
		// ── file copy / cut / paste / delete (system-clipboard + QUndoCommand) ──
		connect(FileOptionCopy, &QAction::triggered, this, [this]() {
			auto* mime = new QMimeData();
			mime->setUrls({ QUrl::fromLocalFile(CurrentFilePath) });
			QByteArray effect;
			QDataStream(&effect, QIODevice::WriteOnly) << (int)1;   // DROPEFFECT_COPY
			mime->setData(QStringLiteral("application/x-qt-windows-mime;value=\"Preferred DropEffect\""), effect);
			QApplication::clipboard()->setMimeData(mime);
			});
		connect(FileOptionCut, &QAction::triggered, this, [this]() {
			auto* mime = new QMimeData();
			mime->setUrls({ QUrl::fromLocalFile(CurrentFilePath) });
			QByteArray effect;
			QDataStream(&effect, QIODevice::WriteOnly) << (int)2;   // DROPEFFECT_MOVE
			mime->setData(QStringLiteral("application/x-qt-windows-mime;value=\"Preferred DropEffect\""), effect);
			QApplication::clipboard()->setMimeData(mime);
			});
		connect(FileOptionPaste, &QAction::triggered, this, [this]() {
			const QMimeData* mime = QApplication::clipboard()->mimeData();
			if (!mime || !mime->hasUrls()) return;

			bool isCut = false;
			if (mime->hasFormat(QStringLiteral("application/x-qt-windows-mime;value=\"Preferred DropEffect\""))) {
				QByteArray data = mime->data(QStringLiteral("application/x-qt-windows-mime;value=\"Preferred DropEffect\""));
				QDataStream stream(data);
				int effect; stream >> effect;
				isCut = (effect == 2);
			}

			const QFileInfo curInfo(CurrentFilePath);
			const QString targetDir = curInfo.isDir() ? CurrentFilePath : curInfo.absolutePath();

			for (const QUrl& url : mime->urls()) {
				const QString src = url.toLocalFile();
				if (src.isEmpty()) continue;

				QString dst = targetDir + "/" + QFileInfo(src).fileName();
				if (dst == src) continue;

				if (QFileInfo::exists(dst)) {
					const QString base = QFileInfo(src).completeBaseName();
					const QString suf = QFileInfo(src).suffix();
					int n = 1;
					do {
						dst = targetDir + "/" + base + " - Copy"
							+ (n > 1 ? QString(" (%1)").arg(n) : QString())
							+ (suf.isEmpty() ? "" : "." + suf);
						++n;
					} while (QFileInfo::exists(dst));
				}

				QUndoCommand* cmd = isCut
					? static_cast<QUndoCommand*>(new FileMoveCommand(src, dst))
					: static_cast<QUndoCommand*>(new FileCopyCommand(src, dst));
				emit fileOperationRequested(cmd);
			}
			});

		connect(FileOptionDelete, &QAction::triggered, this, [this]() {
			if (CurrentFilePath.isEmpty()) return;
			auto projectFilePath = YSSCore::General::YSSProject::getCurrentProject()->getProjectFolder() + "/project.yssp";
			if (QFileInfo(CurrentFilePath).absoluteFilePath() == QFileInfo(projectFilePath).absoluteFilePath()) {
				QMessageBox::warning(this, VITR("YSS::editor.deleteProjectFile.title"), VITR("YSS::editor.deleteProjectFile.message"));
				return;
			}
			auto rtn = QMessageBox::question(this, VITR("YSS::editor.deleteFile.title"), VITR("YSS::editor.deleteFile.message").arg(CurrentFilePath), QMessageBox::Yes | QMessageBox::No);
			if (rtn == QMessageBox::Yes) {
				emit fileOperationRequested(new FileDeleteCommand(CurrentFilePath));
			}
			});

		connect(FileModel, &QFileSystemModel::fileRenamed, this, [this](const QString& path, const QString& oldName, const QString& newName) {
			vgDebug << "ResourceBrowser: otherFileRenamed signal received: " << path << ", " << oldName << ", " << newName; 
			vgDebug << QFileInfo(path + "/" + newName).isFile();
			vgDebug << QFileInfo(path + "/" + newName).isDir();
			if (QFileInfo(path + "/" + newName).isFile()) {
				emit otherFileRenamed(path, oldName, newName);
			}
			else if (QFileInfo(path + "/" + newName).isDir()) {
				emit directoryRenamed(path, oldName, newName);
			}
			});

		connect(this, &ResourceBrowser::otherFileRenamed, this, [this](const QString& path, const QString& oldName, const QString& newName) {
			const QString absOldPath = path + "/" + oldName;
			if (QFileInfo(absOldPath).isFile()) {
				auto editor = YSSFSM->getFileEditWidget(absOldPath);
				if (editor) {
					const QString absNewPath = path + "/" + newName;
					editor->saveFile(absNewPath, true);
				}
			}
			});
		connect(this, &ResourceBrowser::directoryRenamed, this, [this](const QString& path, const QString& oldName, const QString& newName) {
			const QString absOldPath = path + "/" + oldName;
			const QString absNewPath = path + "/" + newName;
			vgDebug << "Directory renamed from" << absOldPath << "to" << absNewPath;
			if (QFileInfo(absNewPath).isDir()) {
				auto newFileAbsPaths = Visindigo::Utility::FileUtility::fileFilter(absNewPath, { "*.*" }, true);
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
		connect(this, &ResourceBrowser::fileOperationRequested, this, [this](QUndoCommand* cmd) {
			cmd->redo();          // execute now
			// TODO: push to global QUndoStack when ready
			delete cmd;           // temporary — replace with stack management later
			this->refresh();
			});

		refreshFileList();
		onThemeChanged();
	}

	ResourceBrowser* ResourceBrowser::getInstance() {
		return Instance;
	}

	ResourceBrowser::~ResourceBrowser() {
		Instance = nullptr;
	}

	void ResourceBrowser::refresh() {
		refreshFileList();
	}

	void ResourceBrowser::setCurrentSelected(const QFileInfo& path) {
		QModelIndex index = FileModel->index(path.absoluteFilePath());
		if (!index.isValid())
			return;
		// 展开祖先节点，确保该项可见
		QModelIndex parent = index.parent();
		while (parent.isValid()) {
			FileTree->expand(parent);
			parent = parent.parent();
		}
		FileTree->setCurrentIndex(index);
		FileTree->scrollTo(index);
	}

	void ResourceBrowser::onNewButtonClicked() {
		QString currentSelectedPath;
		QModelIndex currentIndex = FileTree->currentIndex();
		if (currentIndex.isValid()) {
			currentSelectedPath = FileModel->filePath(currentIndex);
			QFileInfo fileInfo(currentSelectedPath);
			if (fileInfo.isFile()) {
				currentSelectedPath = fileInfo.absoluteDir().absolutePath();
			}
		}
		else {
			currentSelectedPath = ProjectRootDir.path();
		}
		YSS::Editor::MainWin::getInstance()->openNewFileWindow(currentSelectedPath);
	}

	void ResourceBrowser::onThemeChanged() {
		if (TextColor != VISTM->getPaletteTextColor()) {
			TextColor = VISTM->getPaletteTextColor();
			ToolActionRefresh->setIcon(VIApp->getNamedFontIcon("Refresh", 40, { TextColor }));
			ToolActionShrink->setIcon(VIApp->getNamedFontIcon("ChevronUp", 40, { TextColor }));
			ToolActionExpand->setIcon(VIApp->getNamedFontIcon("ChevronDown", 40, { TextColor }));
			ToolActionNewFile->setIcon(VIApp->getNamedFontIcon("SubscriptionAdd", 40, { TextColor }));
			ToolActionNewFolder->setIcon(VIApp->getNamedFontIcon("NewFolder", 40, { TextColor }));
			FileOptionOpen->setIcon(VIApp->getNamedFontIcon("OpenFile", 64, { TextColor }));
			FileOptionRename->setIcon(VIApp->getNamedFontIcon("Rename", 64, { TextColor }));
			FileOptionDelete->setIcon(VIApp->getNamedFontIcon("Delete", 64, { TextColor }));
			FileOptionNewFile->setIcon(VIApp->getNamedFontIcon("SubscriptionAdd", 64, { TextColor }));
			FileOptionNewFolder->setIcon(VIApp->getNamedFontIcon("NewFolder", 64, { TextColor }));
			// These two icons may not satisfy.
			//FileOptionCopyPath->setIcon(VIApp->getNamedFontIcon("HardDrive", 64, { TextColor }));
			//FileOptionCopyName->setIcon(VIApp->getNamedFontIcon("Dictionary", 64, { TextColor }));
			FileOptionCopy->setIcon(VIApp->getNamedFontIcon("Copy", 64, { TextColor }));
			FileOptionPaste->setIcon(VIApp->getNamedFontIcon("Paste", 64, { TextColor }));
			FileOptionCut->setIcon(VIApp->getNamedFontIcon("Cut", 64, { TextColor }));
			FileOptionRefresh->setIcon(VIApp->getNamedFontIcon("Refresh", 64, { TextColor }));
		}
	}

	void ResourceBrowser::refreshFileList() {
		FileModel->setRootPath(ProjectRootDir.path());
		FileTree->setRootIndex(FileModel->index(ProjectRootDir.path()));
	}

	void ResourceBrowser::onItemDoubleClicked(const QModelIndex& index) {
		QModelIndex currentIndex = FileTree->currentIndex();
		if (currentIndex.isValid()) {
			QString filePath = FileModel->filePath(currentIndex);
			QFileInfo fileInfo(filePath);
			if (fileInfo.isFile()) {
				YSSFSM->openFile(filePath);
			}
		}
	}

	void ResourceBrowser::onFileTreeContextMenuRequested(const QPoint& pos) {
		QModelIndex index = FileTree->indexAt(pos);
		const bool onItem = index.isValid();
		if (onItem) {
			CurrentFilePath = FileModel->filePath(index);
			RightClickedIndex = index.siblingAtColumn(0);
			FileTree->setCurrentIndex(RightClickedIndex);
		}
		else {
			// 空处右键：默认选中项目目录本身
			CurrentFilePath = ProjectRootDir.path();
			RightClickedIndex = FileModel->index(ProjectRootDir.path());
			FileTree->setCurrentIndex(RightClickedIndex);
		}

		const QMimeData* clipMime = QApplication::clipboard()->mimeData();
		const bool clipboardHasUrls = clipMime != nullptr && clipMime->hasUrls();

		// 空处右键时只保留“新建文件 / 新建文件夹 / 刷新”可用
		FileOptionOpen->setEnabled(onItem);
		FileOptionRename->setEnabled(onItem);
		FileOptionDelete->setEnabled(onItem);
		FileOptionCopyPath->setEnabled(onItem);
		FileOptionCopyName->setEnabled(onItem);
		FileOptionCopy->setEnabled(onItem);
		FileOptionCut->setEnabled(onItem);
		FileOptionPaste->setEnabled(clipboardHasUrls);
		FileOptionNewFile->setEnabled(true);
		FileOptionNewFolder->setEnabled(true);
		FileOptionRefresh->setEnabled(true);

		FileOptions->exec(FileTree->viewport()->mapToGlobal(pos));
	}

	void ResourceBrowser::onRenameTriggered() {
		if (!RightClickedIndex.isValid())
			return;
		FileTree->scrollTo(RightClickedIndex);
		FileTree->edit(RightClickedIndex);
	}

	void ResourceBrowser::onNewFolderTriggered() {
		QString targetPath;
		QModelIndex currentIndex = FileTree->currentIndex();
		if (currentIndex.isValid()) {
			targetPath = FileModel->filePath(currentIndex);
			QFileInfo fileInfo(targetPath);
			if (fileInfo.isFile()) {
				targetPath = fileInfo.absoluteDir().absolutePath();
			}
		}
		else {
			targetPath = ProjectRootDir.path();
		}

		QDir targetDir(targetPath);
		QStringList existingNames = targetDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

		NewFolderDialog* dialog = new NewFolderDialog();
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->setContext(targetPath, existingNames);
		connect(dialog, &NewFolderDialog::confirmed, this, [this, targetPath](const QString& newName) {
			QDir dir(targetPath);
			if (dir.mkdir(newName)) {
				refreshFileList();
			}
			});
		dialog->show();
	}

	bool ResourceBrowser::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		if (ext == "YSS.MainEditor.ResourceBrowser") {
			return true;
		}
		return false;
	}
}