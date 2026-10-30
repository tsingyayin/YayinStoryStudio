#include <QtCore/qdatastream.h>
#include <QtCore/qmimedata.h>
#include <QtCore/qurl.h>
#include <QtGui/qclipboard.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qtreeview.h>
#include <Editor/FileServerManager.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <General/YSSProject.h>
#include <General/YSSUniqueFileSystem.h>
#include <Widgets/ThemeManager.h>
#include <memory>
#include "Editor/MainEditor/MainWin.h"
#include "Editor/MainEditor/ResourceBrowser.h"
#include "Editor/MainEditor/SimpleFileDialog.h"
#include "Editor/MainEditor/UfsTreeModel.h"
#include "Editor/SSHConnection/SSHConnectionWidget.h"

namespace YSS::Editor {
	static ResourceBrowser* Instance = nullptr;

	namespace {
		constexpr int DropEffectCopy = 1;
		constexpr int DropEffectMove = 2;

		QString dropEffectFormat() {
			return QStringLiteral("application/x-qt-windows-mime;value=\"Preferred DropEffect\"");
		}

		YSSCore::General::YSSFileError notSupportedError(const YSSCore::General::YSSURI& uri) {
			YSSCore::General::YSSFileError error;
			error.Type = YSSCore::General::YSSFileError::ErrorType::NotSupported;
			error.Message = QStringLiteral("No filesystem backend accepts this path");
			error.Detail = uri.toString();
			return error;
		}

		bool clipboardRequestsMove(const QMimeData* mime) {
			if (mime == nullptr || !mime->hasFormat(dropEffectFormat())) {
				return false;
			}
			QDataStream stream(mime->data(dropEffectFormat()));
			int effect = 0;
			stream >> effect;
			return effect == DropEffectMove;
		}

		// 本机文件仍然写一份系统剪贴板，方便直接粘到资源管理器。
		void setClipboardUrls(const YSSCore::General::YSSURI& uri, bool cut) {
			auto* mime = new QMimeData();
			mime->setUrls({ QUrl::fromLocalFile(uri.getLocalPath()) });
			QByteArray effect;
			QDataStream(&effect, QIODevice::WriteOnly) << (cut ? DropEffectMove : DropEffectCopy);
			mime->setData(dropEffectFormat(), effect);
			QApplication::clipboard()->setMimeData(mime);
		}

		// 目标目录已有同名项时生成 “name - Copy”、“name - Copy (2)”。
		QString copyNameIn(const QString& name, const QStringList& existing) {
			QString base = name;
			QString suffix;
			const qsizetype dot = name.lastIndexOf(QLatin1Char('.'));
			if (dot > 0) {
				base = name.left(dot);
				suffix = name.mid(dot);
			}
			QString candidate = base + QStringLiteral(" - Copy") + suffix;
			int index = 2;
			while (existing.contains(candidate, Qt::CaseInsensitive)) {
				candidate = base + QStringLiteral(" - Copy (%1)").arg(index++) + suffix;
			}
			return candidate;
		}
	}

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
		ToolActionRefresh = ToolBar->addAction(VITRL("Visindigo::general.refresh"));
		ToolActionShrink = ToolBar->addAction(VITRL("Visindigo::general.shrink"));
		ToolActionExpand = ToolBar->addAction(VITRL("Visindigo::general.expand"));
		ToolActionNewFile = ToolBar->addAction(VITRL("Visindigo::general.new"));
		ToolActionNewFolder = ToolBar->addAction(VITRL("YSS::menu.file.newFolder"));
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
		FileTree->setHeaderHidden(true);
		FileTree->setUniformRowHeights(true);
		// 双击文件是打开，不是展开目录。
		FileTree->setExpandsOnDoubleClick(false);
		Layout->addWidget(FileTree);

		FileModel = new UfsTreeModel(this);
		FileTree->setModel(FileModel);

		connect(FileTree, &QTreeView::doubleClicked, this, &ResourceBrowser::onItemDoubleClicked);
		connect(FileTree, &QTreeView::customContextMenuRequested, this, &ResourceBrowser::onFileTreeContextMenuRequested);
		connect(FileModel, &UfsTreeModel::directoryFailed, this,
			[this](const YSSCore::General::YSSURI& dir, const YSSCore::General::YSSFileError& error) {
				vgWarningF << "Failed to list" << dir.toString() << error.toString();
				// 远端没登录时直接把连接对话框叫出来，比让目录一直空着好。
				if (error.Type == YSSCore::General::YSSFileError::ErrorType::NeedsCredentials) {
					SSHConnectionWidget::requestVerification(dir);
				}
			});
		// 支持监听的后端（本机）在外部改动后自动刷新，保持以前 QFileSystemModel 的行为。
		connect(YSSCore::General::YSSUniqueFileSystem::getInstance(),
			&YSSCore::General::YSSUniqueFileSystem::directoryChanged, this,
			[this](const YSSCore::General::YSSURI& dir) {
				FileModel->refreshDirectory(dir);
			});
		// 远端登录成功后把之前因为未连接而列不出来的目录补上。
		connect(YSSCore::General::YSSUniqueFileSystem::getInstance(),
			&YSSCore::General::YSSUniqueFileSystem::sshReadyChanged, this,
			[this](const QString& authority, bool ready) {
				Q_UNUSED(authority);
				if (ready) {
					FileModel->refreshAll();
				}
			});

		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		ProjectRootUri = project != nullptr
			? project->getProjectFolder()
			: YSSCore::General::YSSURI::fromLocalPath(QDir::currentPath());
		FileModel->setRoot(ProjectRootUri);

		FileOptions = new QMenu(this);
		FileOptionOpen = FileOptions->addAction(VITRL("Visindigo::general.open"));
		FileOptionRename = FileOptions->addAction(VITRL("Visindigo::general.rename"));
		FileOptionDelete = FileOptions->addAction(VITRL("Visindigo::general.delete"));
		FileOptionNewFile = FileOptions->addAction(VITRL("YSS::menu.file.newFile"));
		FileOptionNewFolder = FileOptions->addAction(VITRL("YSS::menu.file.newFolder"));
		FileOptions->addSeparator();
		FileOptionCopyPath = FileOptions->addAction(VITRL("YSS::menu.edit.copyPath"));
		FileOptionCopyName = FileOptions->addAction(VITRL("YSS::menu.edit.copyName"));
		FileOptions->addSeparator();
		FileOptionCopy = FileOptions->addAction(VITRL("YSS::menu.edit.copy"));
		FileOptionPaste = FileOptions->addAction(VITRL("YSS::menu.edit.paste"));
		FileOptionCut = FileOptions->addAction(VITRL("YSS::menu.edit.cut"));
		FileOptions->addSeparator();
		FileOptionRefresh = FileOptions->addAction(VITRL("Visindigo::general.refresh"));
		FileOptionCompareAsOld = FileOptions->addAction(VITRL("YSS::menu.file.compareAsOld"));
		FileOptionCompareAsNew = FileOptions->addAction(VITRL("YSS::menu.file.compareAsNew"));

		connect(FileOptionOpen, &QAction::triggered, this, [this]() {
			if (!CurrentUri.isValid()) {
				return;
			}
			const QModelIndex index = RightClickedIndex;
			if (FileModel->isDirectory(index)) {
				if (index.isValid()) {
					FileTree->setExpanded(index, !FileTree->isExpanded(index));
				}
				return;
			}
			YSSFSM->openFile(CurrentUri);
			});
		connect(FileOptionNewFile, &QAction::triggered, this, &ResourceBrowser::onNewButtonClicked);
		connect(FileOptionNewFolder, &QAction::triggered, this, &ResourceBrowser::onNewFolderTriggered);
		connect(FileOptionRename, &QAction::triggered, this, &ResourceBrowser::onRenameTriggered);
		connect(FileOptionDelete, &QAction::triggered, this, &ResourceBrowser::onDeleteTriggered);
		connect(FileOptionRefresh, &QAction::triggered, this, &ResourceBrowser::refreshFileList);
		connect(FileOptionCopy, &QAction::triggered, this, [this]() {
			onCopyTriggered(false);
			});
		connect(FileOptionCut, &QAction::triggered, this, [this]() {
			onCopyTriggered(true);
			});
		connect(FileOptionPaste, &QAction::triggered, this, &ResourceBrowser::onPasteTriggered);
		connect(FileOptionCompareAsOld, &QAction::triggered, this, [this]() {
			MainWin::getInstance()->setCompareFileAsOld(CurrentUri);
			});
		connect(FileOptionCompareAsNew, &QAction::triggered, this, [this]() {
			MainWin::getInstance()->setCompareFileAsNew(CurrentUri);
			});
		connect(FileOptionCopyPath, &QAction::triggered, this, [this]() {
			QApplication::clipboard()->setText(CurrentUri.toString());
			});
		connect(FileOptionCopyName, &QAction::triggered, this, [this]() {
			QApplication::clipboard()->setText(CurrentUri.getFileName());
			});

		refreshFileList();
		onThemeChanged();

		// 重命名对话框归主窗口管，这里只负责发起请求。
		if (MainWin* win = MainWin::getInstance()) {
			connect(this, &ResourceBrowser::renameRequested, win, &MainWin::showRenameDialog,
				Qt::UniqueConnection);
		}
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

	void ResourceBrowser::refreshFileList() {
		FileModel->refreshAll();
	}

	void ResourceBrowser::setCurrentSelected(const YSSCore::General::YSSURI& uri) {
		if (!uri.isValid()) {
			return;
		}
		FileModel->reveal(uri, [this](const QModelIndex& index) {
			if (index.isValid()) {
				selectAndExpand(index);
			}
			});
	}

	void ResourceBrowser::selectAndExpand(const QModelIndex& index) {
		for (QModelIndex parent = index.parent(); parent.isValid(); parent = parent.parent()) {
			FileTree->expand(parent);
		}
		FileTree->setCurrentIndex(index);
		FileTree->scrollTo(index);
	}

	void ResourceBrowser::onNewButtonClicked() {
		const YSSCore::General::YSSURI dir = targetDirectory();
		if (!dir.isValid()) {
			return;
		}
		if (MainWin* win = MainWin::getInstance()) {
			win->openNewFileWindow(dir);
		}
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
			FileOptionCopy->setIcon(VIApp->getNamedFontIcon("Copy", 64, { TextColor }));
			FileOptionPaste->setIcon(VIApp->getNamedFontIcon("Paste", 64, { TextColor }));
			FileOptionCut->setIcon(VIApp->getNamedFontIcon("Cut", 64, { TextColor }));
			FileOptionRefresh->setIcon(VIApp->getNamedFontIcon("Refresh", 64, { TextColor }));
		}
	}

	void ResourceBrowser::onItemDoubleClicked(const QModelIndex& index) {
		if (!index.isValid()) {
			return;
		}
		if (FileModel->isDirectory(index)) {
			FileTree->setExpanded(index, !FileTree->isExpanded(index));
			return;
		}
		const YSSCore::General::YSSURI uri = FileModel->uriOf(index);
		if (uri.isValid()) {
			YSSFSM->openFile(uri);
		}
	}

	void ResourceBrowser::onFileTreeContextMenuRequested(const QPoint& pos) {
		const QModelIndex index = FileTree->indexAt(pos);
		const bool onItem = index.isValid();
		if (onItem) {
			CurrentUri = FileModel->uriOf(index);
			RightClickedIndex = index;
			FileTree->setCurrentIndex(index);
		}
		else {
			// 空处右键：所有"新建/粘贴"都落到项目根目录。
			CurrentUri = ProjectRootUri;
			RightClickedIndex = QModelIndex();
			FileTree->setCurrentIndex(QModelIndex());
		}

		const bool isDir = onItem ? FileModel->isDirectory(index) : true;
		bool clipboardHasFiles = !ClipboardUris.isEmpty();
		if (!clipboardHasFiles) {
			const QMimeData* mime = QApplication::clipboard()->mimeData();
			if (mime != nullptr) {
				for (const QUrl& url : mime->urls()) {
					if (url.isLocalFile()) {
						clipboardHasFiles = true;
						break;
					}
				}
			}
		}

		FileOptionOpen->setEnabled(onItem);
		FileOptionRename->setEnabled(onItem);
		FileOptionDelete->setEnabled(onItem);
		FileOptionCopyPath->setEnabled(CurrentUri.isValid());
		FileOptionCopyName->setEnabled(onItem);
		FileOptionCopy->setEnabled(onItem);
		FileOptionCut->setEnabled(onItem);
		FileOptionPaste->setEnabled(clipboardHasFiles);
		FileOptionNewFile->setEnabled(true);
		FileOptionNewFolder->setEnabled(true);
		FileOptionRefresh->setEnabled(true);
		const bool canCompare = onItem && !isDir;
		FileOptionCompareAsOld->setEnabled(canCompare);
		FileOptionCompareAsNew->setEnabled(canCompare);

		FileOptions->exec(FileTree->viewport()->mapToGlobal(pos));
	}

	YSSCore::General::YSSURI ResourceBrowser::targetDirectory() const {
		const QModelIndex index = FileTree->currentIndex();
		if (!index.isValid()) {
			return ProjectRootUri;
		}
		const YSSCore::General::YSSURI uri = FileModel->uriOf(index);
		if (!uri.isValid()) {
			return ProjectRootUri;
		}
		return FileModel->isDirectory(index) ? uri : uri.parent();
	}

	void ResourceBrowser::onRenameTriggered() {
		if (!CurrentUri.isValid() || !RightClickedIndex.isValid()) {
			return;
		}
		emit renameRequested(CurrentUri, FileModel->isDirectory(RightClickedIndex));
	}

	void ResourceBrowser::onNewFolderTriggered() {
		const YSSCore::General::YSSURI dir = targetDirectory();
		if (!dir.isValid()) {
			return;
		}
		NewFolderDialog* dialog = new NewFolderDialog();
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->setContext(dir, FileModel->knownChildNames(dir));
		connect(dialog, &NewFolderDialog::confirmed, this, [this, dir](const QString& newName) {
			const YSSCore::General::YSSURI target = dir.resolve(newName);
			YSSCore::General::YSSFileSystemReply* reply =
				YSSCore::General::YSSUniqueFileSystem::getInstance()->mkdir(target);
			if (reply == nullptr) {
				showOperationError(target, notSupportedError(target));
				return;
			}
			connect(reply, &YSSCore::General::YSSFileSystemReply::succeeded, this, [this, dir]() {
				FileModel->refreshDirectory(dir);
				});
			connect(reply, &YSSCore::General::YSSFileSystemReply::failed, this,
				[this, target](const YSSCore::General::YSSFileError& error) {
					showOperationError(target, error);
				});
			});
		dialog->show();
	}

	void ResourceBrowser::onDeleteTriggered() {
		const YSSCore::General::YSSURI uri = CurrentUri;
		if (!uri.isValid() || !RightClickedIndex.isValid()) {
			return;
		}
		if (uri == ProjectRootUri.resolve(QStringLiteral("project.yssp"))) {
			QMessageBox::warning(this, VITRL("YSS::editor.deleteProjectFile.title"),
				VITRL("YSS::editor.deleteProjectFile.message"));
			return;
		}
		YSSCore::General::YSSUniqueFileSystem* fileSystem = YSSCore::General::YSSUniqueFileSystem::getInstance();
		const bool canRecycle = fileSystem->getCapabilities(uri)
			.testFlag(YSSCore::General::YSSUniqueFileSystem::Capability::CanRecycle);
		const QString question = canRecycle
			? VITRL("YSS::editor.deleteFile.message").arg(uri.toString())
			: VITRL("YSS::editor.deleteFile.permanentMessage").arg(uri.toString());
		if (QMessageBox::question(this, VITRL("YSS::editor.deleteFile.title"), question,
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
			return;
		}
		YSSCore::General::YSSFileSystemReply* reply = canRecycle
			? fileSystem->recycle(uri)
			: fileSystem->remove(uri);
		if (reply == nullptr) {
			showOperationError(uri, notSupportedError(uri));
			return;
		}
		connect(reply, &YSSCore::General::YSSFileSystemReply::succeeded, this, [this, uri]() {
			refreshParentOf(uri);
			});
		connect(reply, &YSSCore::General::YSSFileSystemReply::failed, this,
			[this, uri](const YSSCore::General::YSSFileError& error) {
				showOperationError(uri, error);
			});
	}

	void ResourceBrowser::onCopyTriggered(bool cut) {
		if (!CurrentUri.isValid() || !RightClickedIndex.isValid()) {
			return;
		}
		ClipboardUris = { CurrentUri };
		ClipboardCut = cut;
		if (CurrentUri.isLocal()) {
			setClipboardUrls(CurrentUri, cut);
		}
	}

	void ResourceBrowser::onPasteTriggered() {
		const YSSCore::General::YSSURI dir = targetDirectory();
		if (!dir.isValid()) {
			return;
		}

		// 内部剪贴板负责 ssh:// 这种系统剪贴板装不下的 URI，同时仍然接受从资源管理器复制的本地文件。
		QList<QPair<YSSCore::General::YSSURI, bool>> sources;
		for (const YSSCore::General::YSSURI& source : ClipboardUris) {
			sources.append(qMakePair(source, ClipboardCut));
		}
		if (const QMimeData* mime = QApplication::clipboard()->mimeData()) {
			const bool systemCut = clipboardRequestsMove(mime);
			for (const QUrl& url : mime->urls()) {
				if (!url.isLocalFile()) {
					continue;
				}
				const YSSCore::General::YSSURI source =
					YSSCore::General::YSSURI::fromLocalPath(url.toLocalFile());
				if (!source.isValid()) {
					continue;
				}
				bool duplicated = false;
				for (const QPair<YSSCore::General::YSSURI, bool>& existing : sources) {
					if (existing.first == source) {
						duplicated = true;
						break;
					}
				}
				if (!duplicated) {
					sources.append(qMakePair(source, systemCut));
				}
			}
		}
		if (sources.isEmpty()) {
			return;
		}

		const bool cut = ClipboardCut;
		QStringList existingNames = FileModel->knownChildNames(dir);
		// 一次粘贴可能提交多条请求：全部结束（成功或失败）后统一刷新一次目录。
		auto remaining = std::make_shared<int>(0);
		auto hadError = std::make_shared<bool>(false);
		auto finishOne = [this, dir, remaining, hadError, cut](bool failed) {
			if (failed) {
				*hadError = true;
			}
			if (--(*remaining) > 0) {
				return;
			}
			FileModel->refreshDirectory(dir);
			if (cut && !*hadError) {
				ClipboardUris.clear();
				ClipboardCut = false;
			}
		};
		YSSCore::General::YSSUniqueFileSystem* fileSystem = YSSCore::General::YSSUniqueFileSystem::getInstance();
		for (const QPair<YSSCore::General::YSSURI, bool>& entry : sources) {
			const YSSCore::General::YSSURI source = entry.first;
			QString name = source.getFileName();
			if (name.isEmpty()) {
				continue;
			}
			YSSCore::General::YSSURI target = dir.resolve(name);
			if (target == source) {
				continue;
			}
			if (existingNames.contains(name, Qt::CaseInsensitive)) {
				name = copyNameIn(name, existingNames);
				target = dir.resolve(name);
			}
			existingNames.append(name);

			YSSCore::General::YSSFileSystemReply* reply = entry.second
				? fileSystem->move(source, target)
				: fileSystem->copy(source, target);
			if (reply == nullptr) {
				showOperationError(target, notSupportedError(target));
				continue;
			}
			++(*remaining);
			connect(reply, &YSSCore::General::YSSFileSystemReply::succeeded, this,
				[finishOne]() {
					finishOne(false);
				});
			connect(reply, &YSSCore::General::YSSFileSystemReply::failed, this,
				[this, target, finishOne](const YSSCore::General::YSSFileError& error) {
					showOperationError(target, error);
					finishOne(true);
				});
		}
	}

	void ResourceBrowser::refreshParentOf(const YSSCore::General::YSSURI& uri) {
		const YSSCore::General::YSSURI parent = uri.parent();
		if (parent.isValid()) {
			FileModel->refreshDirectory(parent);
		}
	}

	void ResourceBrowser::showOperationError(const YSSCore::General::YSSURI& uri,
		const YSSCore::General::YSSFileError& error) {
		vgWarningF << "Resource browser operation failed on" << uri.toString() << error.toString();
		QMessageBox::warning(this,
			VITRL("YSS::editor.fileOperation.failed.title"),
			VITRL("YSS::editor.fileOperation.failed.message").arg(uri.getFileName()));
	}

	bool ResourceBrowser::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		Q_UNUSED(fileName);
		Q_UNUSED(param);
		if (ext == "YSS.MainEditor.ResourceBrowser") {
			return true;
		}
		return false;
	}
}
