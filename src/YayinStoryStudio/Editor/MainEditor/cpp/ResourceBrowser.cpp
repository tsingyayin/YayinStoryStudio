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
namespace YSS::Editor {
	ResourceBrowser::ResourceBrowser(QWidget* parent) :QWidget(parent) {
		ButtonWidget = new QWidget(this);

		ButtonLayout = new QHBoxLayout(ButtonWidget);
		RefreshButton = new QToolButton(ButtonWidget);
		RefreshButton->setIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon/refresh.png"));
		RefreshButton->setToolTip(VITR("Visindigo::general.refresh"));
		RefreshButton->setIconSize(QSize(32, 32));
		RefreshButton->setFixedSize(40, 40);
	
		ShrinkButton = new QToolButton(ButtonWidget);
		ShrinkButton->setIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon/shrink.png"));
		ShrinkButton->setToolTip(VITR("Visindigo::general.shrink"));
		ShrinkButton->setIconSize(QSize(30, 30));
		ShrinkButton->setFixedSize(40, 40);
		ExpandButton = new QToolButton(ButtonWidget);
		ExpandButton->setIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon/expand.png"));
		ExpandButton->setToolTip(VITR("Visindigo::general.expand"));
		ExpandButton->setIconSize(QSize(30, 30));
		ExpandButton->setFixedSize(40, 40);
		NewButton = new QToolButton(ButtonWidget);
		NewButton->setIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon/new.png"));
		NewButton->setToolTip(VITR("Visindigo::general.new"));
		NewButton->setIconSize(QSize(30, 30));
		NewButton->setFixedSize(40, 40);
		ButtonLayout->addWidget(RefreshButton);
		ButtonLayout->addWidget(ShrinkButton);
		ButtonLayout->addWidget(ExpandButton);
		ButtonLayout->addWidget(NewButton);
		ButtonLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		ButtonLayout->setContentsMargins(0, 0, 0, 0);
		ButtonWidget->setLayout(ButtonLayout);
		ButtonWidget->setMinimumHeight(40);

		Layout = new QVBoxLayout(this);
		FileTree = new QTreeView(this);
		FileModel = new QFileSystemModel(this);
		FileTree->setModel(FileModel);
		FileTree->setHeaderHidden(true);
		Layout->addWidget(ButtonWidget);
		Layout->addWidget(FileTree);

		connect(RefreshButton, &QToolButton::clicked, this, &ResourceBrowser::refreshFileList);
		connect(NewButton, &QToolButton::clicked, this, &ResourceBrowser::onNewButtonClicked);
		connect(FileTree, &QTreeView::doubleClicked, this, &ResourceBrowser::onItemDoubleClicked);
		connect(ShrinkButton, &QToolButton::clicked, this, [this]() {
			FileTree->collapseAll();
			});
		connect(ExpandButton, &QToolButton::clicked, this, [this]() {
			FileTree->expandAll();
			});
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
}