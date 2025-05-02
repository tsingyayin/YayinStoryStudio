#include "../ResourceBrowser.h"

namespace YSS::Editor {
	ResourceBrowser::ResourceBrowser(QWidget* parent):QWidget(parent) {
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

		CurrentDir.setPath("D:/ASE/ASE 2.04.23.0/Arknights_StoryEditor_Data/StreamingAssets/Stories");
		CurrentPath->setText(CurrentDir.path());

		connect(BackButton, &QPushButton::clicked, this, &ResourceBrowser::onBackButtonClicked);
		connect(RefreshButton, &QPushButton::clicked, this, &ResourceBrowser::refreshFileList);
		connect(FileList, &QListWidget::itemDoubleClicked, this, &ResourceBrowser::onItemDoubleClicked);
		refreshFileList();
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
			item->setIcon(QIcon(":/icons/file.png"));
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
			emit openFile(filePath);
		}
	}
}