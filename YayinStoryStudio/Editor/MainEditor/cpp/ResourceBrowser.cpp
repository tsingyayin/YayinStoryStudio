#include "../ResourceBrowser.h"
#include <Editor/FileServerManager.h>
#include <Editor/FileTemplateManager.h>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include "../../GlobalValue.h"
#include <General/YSSProject.h>
#include "../../NewFilePage/NewFileWin.h"

namespace YSS::Editor {
	ResourceBrowser::ResourceBrowser(QWidget* parent) :QWidget(parent) {
		ButtonWidget = new QWidget(this);

		ButtonLayout = new QHBoxLayout(ButtonWidget);
		RefreshButton = new QPushButton("Refresh", ButtonWidget);
		NewButton = new QPushButton("New", ButtonWidget);
		BackButton = new QPushButton("Back", ButtonWidget);
		ButtonLayout->addWidget(RefreshButton);
		ButtonLayout->addWidget(NewButton);
		ButtonLayout->addWidget(BackButton);
		ButtonWidget->setLayout(ButtonLayout);

		Layout = new QVBoxLayout(this);
		CurrentPath = new QLineEdit(this);
		FileList = new QListWidget(this);
		Layout->addWidget(ButtonWidget);
		Layout->addWidget(CurrentPath);
		Layout->addWidget(FileList);

		YSSCore::General::YSSProject* project = GlobalValue::getCurrentProject();
		if (project != nullptr) {
			CurrentDir.setPath(project->getProjectFolder());
		}
		else {
			CurrentDir.setPath(QDir::currentPath());
		}
		CurrentPath->setText(CurrentDir.path());

		connect(BackButton, &QPushButton::clicked, this, &ResourceBrowser::onBackButtonClicked);
		connect(RefreshButton, &QPushButton::clicked, this, &ResourceBrowser::refreshFileList);
		connect(FileList, &QListWidget::itemDoubleClicked, this, &ResourceBrowser::onItemDoubleClicked);
		connect(NewButton, &QPushButton::clicked, this, &ResourceBrowser::onNewButtonClicked);
		refreshFileList();
	}

	void ResourceBrowser::onNewButtonClicked() {
		YSS::NewFilePage::NewFileWin* newFileWin = new YSS::NewFilePage::NewFileWin(CurrentPath->text());
		newFileWin->setAttribute(Qt::WA_DeleteOnClose);
		newFileWin->setWindowModality(Qt::ApplicationModal);
		connect(newFileWin, &YSS::NewFilePage::NewFileWin::filePrepared, this, [this](const QString& filePath) {
			if (QFileInfo(filePath).isFile()) {
				YSSFSM->openFile(filePath);
			}
			});
		newFileWin->show();
	}
	void ResourceBrowser::onBackButtonClicked() {
		if (CurrentDir.cdUp()) {
			CurrentPath->setText(CurrentDir.path());
			refreshFileList();
		}
	}

	void ResourceBrowser::refreshFileList() {
		FileList->clear();
		QDir dir(CurrentDir.path());
		//先筛文件夹
		QStringList folders = dir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
		for (const QString& folder : folders) {
			QListWidgetItem* item = new QListWidgetItem(folder);
			item->setIcon(QIcon(":/icons/folder.png"));
			FileList->addItem(item);
		}
		//再筛文件
		QStringList files = dir.entryList(QDir::NoDotAndDotDot | QDir::Files);
		for (const QString& file : files) {
			QListWidgetItem* item = new QListWidgetItem(file);
			//if image load it as icon
			if (file.endsWith(".png") || file.endsWith(".jpg") || file.endsWith(".jpeg")) {
				item->setIcon(QIcon(dir.absoluteFilePath(file)));
			}
			else {
				item->setIcon(QIcon(":/icons/file.png"));
			}
			FileList->addItem(item);
		}
	}

	void ResourceBrowser::onItemDoubleClicked(QListWidgetItem* item) {
		QString filePath = CurrentDir.absoluteFilePath(item->text());
		if (QFileInfo(filePath).isDir()) {
			CurrentDir.cd(item->text());
			CurrentPath->setText(CurrentDir.path());
			refreshFileList();
		}
		else {
			YSSFSM->openFile(filePath);
		}
	}
}