#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qfilesystemmodel.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qtoolbar.h>
#include <QtGui/qpainter.h>
#include <General/TranslationHost.h>
#include <General/Log.h>
#include <General/VIApplication.h>
#include <General/YSSProject.h>
#include <Editor/FileServerManager.h>
#include <Editor/FileTemplateManager.h>
#include "Editor/MainEditor/ResourceBrowser.h"
#include "Editor/NewFilePage/NewFileWin.h"
#include "Editor/GlobalValue.h"

namespace YSS::Editor {
	ResourceBrowser::ResourceBrowser(QWidget* parent) :Visindigo::Widgets::BorderFrame(parent) {
		Layout = new QVBoxLayout(this);
		Layout->setSpacing(0);
		Layout->setContentsMargins(0, 0, 0, 0);

		ToolBar = new QToolBar(this);
		ToolActionRefresh = ToolBar->addAction(VITR("Visindigo::general.refresh"));
		ToolActionShrink = ToolBar->addAction(VITR("Visindigo::general.shrink"));
		ToolActionExpand = ToolBar->addAction(VITR("Visindigo::general.expand"));
		ToolActionNew = ToolBar->addAction(VITR("Visindigo::general.new"));
		ToolBar->setMaximumHeight(40);
		Layout->addWidget(ToolBar);

		FileTree = new QTreeView(this);
		FileTree->setContextMenuPolicy(Qt::CustomContextMenu);
		Layout->addWidget(FileTree);
		
		FileModel = new QFileSystemModel(this);
		FileTree->setModel(FileModel);
		FileTree->setHeaderHidden(true);
		
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
		connect(ToolActionRefresh, &QAction::triggered, this, &ResourceBrowser::refreshFileList);
		connect(ToolActionNew, &QAction::triggered, this, &ResourceBrowser::onNewButtonClicked);
		connect(FileTree, &QTreeView::doubleClicked, this, &ResourceBrowser::onItemDoubleClicked);
		connect(ToolActionShrink, &QAction::triggered, this, [this]() {
			FileTree->collapseAll();
			});
		connect(ToolActionExpand, &QAction::triggered, this, [this]() {
			FileTree->expandAll();
			});
		connect(FileTree, &QTreeView::customContextMenuRequested, this, &ResourceBrowser::onFileTreeContextMenuRequested);

		connect(FileOptionNewFile, &QAction::triggered, this, &ResourceBrowser::onNewButtonClicked);

		setColorfulEnable(true);
		onThemeChanged();
	}

	void ResourceBrowser::openNewFileWindow() {
		onNewButtonClicked();
	}

	void ResourceBrowser::refresh() {
		refreshFileList();
	}

	void ResourceBrowser::showEvent(QShowEvent* event) {
		YSSCore::General::YSSProject* project = GlobalValue::getCurrentProject();
		if (project != nullptr) {
			CurrentDir.setPath(project->getProjectFolder());
			FileTree->setRootIndex(FileModel->setRootPath(project->getProjectFolder()));
			for (int i = 1; i < FileModel->columnCount(); i++) {
				FileTree->setColumnHidden(i, true);
			}
		}
		else {
			CurrentDir.setPath(QDir::currentPath());
			FileTree->setRootIndex(FileModel->setRootPath(QDir::currentPath()));
			for (int i = 1; i < FileModel->columnCount(); i++) {
				FileTree->setColumnHidden(i, true);
			}
		}
		refreshFileList();
		emit visibilityChanged(true);
	}

	void ResourceBrowser::hideEvent(QHideEvent* event) {
		emit visibilityChanged(false);
	}

	void ResourceBrowser::resizeEvent(QResizeEvent* event) {
		if (event->size().width() == 0 || event->size().height() == 0) {
			emit visibilityChanged(false);
		}
		else {
			emit visibilityChanged(true);
		}
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
			currentSelectedPath = CurrentDir.path();
		}
		YSS::NewFilePage::NewFileWin* newFileWin = new YSS::NewFilePage::NewFileWin(currentSelectedPath);
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

	void ResourceBrowser::onThemeChanged() {
		if (TextColor != VISTM->getPaletteTextColor()) {
			TextColor = VISTM->getPaletteTextColor();
			ToolActionRefresh->setIcon(VIApp->getNamedFontIcon("Refresh", 40, { TextColor }));
			ToolActionShrink->setIcon(VIApp->getNamedFontIcon("ChevronUp", 40, { TextColor }));
			ToolActionExpand->setIcon(VIApp->getNamedFontIcon("ChevronDown", 40, { TextColor }));
			ToolActionNew->setIcon(VIApp->getNamedFontIcon("SubscriptionAdd", 40, { TextColor }));
			FileOptionOpen->setIcon(VIApp->getNamedFontIcon("OpenFile", 64, { TextColor }));
			FileOptionRename->setIcon(VIApp->getNamedFontIcon("Rename", 64, { TextColor }));
			FileOptionDelete->setIcon(VIApp->getNamedFontIcon("Delete", 64, { TextColor }));
			FileOptionNewFile->setIcon(VIApp->getNamedFontIcon("SubscriptionAdd", 64, { TextColor }));
			FileOptionNewFolder->setIcon(VIApp->getNamedFontIcon("NewFolder", 64, { TextColor }));
			FileOptionCopyPath->setIcon(VIApp->getNamedFontIcon("HardDrive", 64, { TextColor }));
			FileOptionCopyName->setIcon(VIApp->getNamedFontIcon("Dictionary", 64, { TextColor }));
			FileOptionCopy->setIcon(VIApp->getNamedFontIcon("Copy", 64, { TextColor }));
			FileOptionPaste->setIcon(VIApp->getNamedFontIcon("Paste", 64, { TextColor }));
			FileOptionCut->setIcon(VIApp->getNamedFontIcon("Cut", 64, { TextColor }));
		}
	}

	void ResourceBrowser::refreshFileList() {
		FileModel->setRootPath(CurrentDir.path());
		FileTree->setRootIndex(FileModel->index(CurrentDir.path()));
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

	void ResourceBrowser::paintEvent(QPaintEvent* event) {
		Visindigo::Widgets::BorderFrame::paintEvent(event);
	}

	void ResourceBrowser::onFileTreeContextMenuRequested(const QPoint& pos) {
		QModelIndex index = FileTree->indexAt(pos);
		if (!index.isValid())
			return;
		CurrentFilePath = FileModel->filePath(index); 
		FileOptions->exec(FileTree->viewport()->mapToGlobal(pos));
	}
}