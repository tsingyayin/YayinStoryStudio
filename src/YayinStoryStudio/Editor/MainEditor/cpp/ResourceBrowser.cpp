#include "../ResourceBrowser.h"
#include <Editor/FileServerManager.h>
#include <Editor/FileTemplateManager.h>
#include <QHBoxLayout>
#include <QPushButton>
#include <QtWidgets/qtoolbutton.h>
#include <QListWidget>
#include <QLineEdit>
#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qfilesystemmodel.h>
#include "Editor/GlobalValue.h"
#include <General/YSSProject.h>
#include "Editor/NewFilePage/NewFileWin.h"
#include <General/TranslationHost.h>
#include <General/Log.h>
#include <QtGui/qevent.h>
#include <General/VIApplication.h>
#include <QtWidgets/qtoolbar.h>
#include <QtGui/qpainter.h>
namespace YSS::Editor {
	ResourceBrowser::ResourceBrowser(QWidget* parent) :Visindigo::Widgets::BorderFrame(parent) {
		Layout = new QVBoxLayout(this);
		Layout->setSpacing(0);
		Layout->setContentsMargins(0, 0, 0, 0);
		//ButtonWidget = new QWidget(this);

		//ButtonLayout = new QHBoxLayout(ButtonWidget);

		ToolBar = new QToolBar(this);
		ToolActionRefresh = ToolBar->addAction(VITR("Visindigo::general.refresh"));
		ToolActionShrink = ToolBar->addAction(VITR("Visindigo::general.shrink"));
		ToolActionExpand = ToolBar->addAction(VITR("Visindigo::general.expand"));
		ToolActionNew = ToolBar->addAction(VITR("Visindigo::general.new"));
		ToolBar->setMaximumHeight(40);
		Layout->addWidget(ToolBar);
		//ButtonLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		//ButtonLayout->setContentsMargins(0, 0, 0, 0);
		//ButtonWidget->setLayout(ButtonLayout);
		//ButtonWidget->setMinimumHeight(40);

		
		FileTree = new QTreeView(this);
		FileModel = new QFileSystemModel(this);
		FileTree->setModel(FileModel);
		FileTree->setHeaderHidden(true);

		Layout->addWidget(FileTree);

		connect(ToolActionRefresh, &QAction::triggered, this, &ResourceBrowser::refreshFileList);
		connect(ToolActionNew, &QAction::triggered, this, &ResourceBrowser::onNewButtonClicked);
		connect(FileTree, &QTreeView::doubleClicked, this, &ResourceBrowser::onItemDoubleClicked);
		connect(ToolActionShrink, &QAction::triggered, this, [this]() {
			FileTree->collapseAll();
			});
		connect(ToolActionExpand, &QAction::triggered, this, [this]() {
			FileTree->expandAll();
			});

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
			ToolActionRefresh->setIcon(VIApp->getFontIcon("\uE72C", 40, { TextColor }));
			ToolActionShrink->setIcon(VIApp->getFontIcon("\uE70E", 40, { TextColor }));
			ToolActionExpand->setIcon(VIApp->getFontIcon("\uE70D", 40, {TextColor }));
			ToolActionNew->setIcon(VIApp->getFontIcon("\uED11", 40, { TextColor }));
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
}