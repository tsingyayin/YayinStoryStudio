#include "../ResourceBrowser.h"
#include <Editor/FileServerManager.h>
#include <Editor/FileTemplateManager.h>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qfilesystemmodel.h>
#include "Editor/GlobalValue.h"
#include <General/YSSProject.h>
#include "Editor/NewFilePage/NewFileWin.h"
#include <General/TranslationHost.h>
namespace YSS::Editor {
	ResourceBrowser::ResourceBrowser(QWidget* parent) :QWidget(parent) {
		ButtonWidget = new QWidget(this);

		ButtonLayout = new QHBoxLayout(ButtonWidget);
		RefreshButton = new QPushButton(VITR("Visindigo::general.refresh"), ButtonWidget);
		NewButton = new QPushButton(VITR("Visindigo::general.new"), ButtonWidget);
		ButtonLayout->addWidget(RefreshButton);
		ButtonLayout->addWidget(NewButton);
		ButtonLayout->setContentsMargins(0, 0, 0, 0);
		ButtonWidget->setLayout(ButtonLayout);

		Layout = new QVBoxLayout(this);
		FileTree = new QTreeView(this);
		FileModel = new QFileSystemModel(this);
		FileTree->setModel(FileModel);
		FileTree->setHeaderHidden(true);
		Layout->addWidget(ButtonWidget);
		Layout->addWidget(FileTree);

		connect(RefreshButton, &QPushButton::clicked, this, &ResourceBrowser::refreshFileList);
		connect(NewButton, &QPushButton::clicked, this, &ResourceBrowser::onNewButtonClicked);
		connect(FileTree, &QTreeView::doubleClicked, this, &ResourceBrowser::onItemDoubleClicked);
	}

	void ResourceBrowser::openNewFileWindow() {
		onNewButtonClicked();
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
	}

	void ResourceBrowser::onNewButtonClicked() {
		QString currentSelectedPath;
		QModelIndex currentIndex = FileTree->currentIndex();
		if (currentIndex.isValid()) {
			currentSelectedPath = FileModel->filePath(currentIndex);
		}
		else {
			currentSelectedPath = CurrentDir.path();
		}
		YSS::NewFilePage::NewFileWin* newFileWin = new YSS::NewFilePage::NewFileWin(currentSelectedPath);
		newFileWin->setAttribute(Qt::WA_DeleteOnClose);
		newFileWin->setWindowModality(Qt::ApplicationModal);
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