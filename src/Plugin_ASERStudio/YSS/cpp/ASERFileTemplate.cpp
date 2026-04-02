#include "YSS/ASERFileTemplate.h"
#include <QtWidgets/qboxlayout.h>
#include <General/TranslationHost.h>
#include <Widgets/ConfigWidget.h>
#include <Utility/FileUtility.h>
#include <QtWidgets/qpushbutton.h>
#include <Utility/JsonConfig.h>
#include <QtWidgets/qlabel.h>
#include <General/YSSProject.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qfile.h>

namespace ASERStudio::YSS {
	class FileTemplateInitWidget_AStoryXPrivate {
		friend class FileTemplateInitWidget_AStoryX;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
		QLabel* WhereLabel = nullptr;
		QPushButton* CreateButton = nullptr;
		QVBoxLayout* Layout = nullptr;
		QHBoxLayout* ButtonLayout = nullptr;
		QString FilePath;
		QString FileName;
	};

	FileTemplateInitWidget_AStoryX::FileTemplateInitWidget_AStoryX(const QString& initFolder, QWidget* parent)
		: YSSCore::Editor::FileTemplateInitWidget(initFolder, parent)
	{
		d = new FileTemplateInitWidget_AStoryXPrivate;
		this->setMinimumWidth(800);
		this->setWindowTitle(VITR("ASERStudio::fileProvider.window.title"));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget(this);
		d->ConfigWidget->loadCWJson(Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.aserstudio/configWidget/FTP.json"));
		d->ConfigWidget->setLineEditText("File.Path", getInitFolder());
		d->Layout = new QVBoxLayout(this);
		d->Layout->setContentsMargins(0, 0, 0, 0);
		d->Layout->addWidget(d->ConfigWidget);
		d->ButtonLayout = new QHBoxLayout();
		d->ButtonLayout->setContentsMargins(10, 0, 10, 0);
		d->Layout->addLayout(d->ButtonLayout);
		d->WhereLabel = new QLabel(this);
		d->WhereLabel->setWordWrap(true);
		d->ButtonLayout->addWidget(d->WhereLabel);
		d->CreateButton = new QPushButton(VITR("ASERStudio::provider.window.create"), this);
		d->CreateButton->setObjectName("CreateButton");
		d->CreateButton->setMinimumWidth(120);
		d->CreateButton->setMaximumWidth(160);
		d->ButtonLayout->addWidget(d->CreateButton);
		d->FilePath = d->ConfigWidget->getConfig()->getString("File.Path");
		d->FileName = d->ConfigWidget->getConfig()->getString("File.Name");
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::lineEditTextChanged, this, &FileTemplateInitWidget_AStoryX::onLineEditTextChanged);
		connect(d->CreateButton, &QPushButton::clicked, this, &FileTemplateInitWidget_AStoryX::onCreateButtonClicked);
		refreshWhereLabel();
	}

	FileTemplateInitWidget_AStoryX::~FileTemplateInitWidget_AStoryX() {
		delete d;
	}

	void FileTemplateInitWidget_AStoryX::resizeEvent(QResizeEvent* event) {
		YSSCore::Editor::FileTemplateInitWidget::resizeEvent(event);
	}

	void FileTemplateInitWidget_AStoryX::onLineEditTextChanged(const QString& node, const QString& text) {
		if (node == "File.Path") {
			d->FilePath = text;
		}
		else if (node == "File.Name") {
			d->FileName = text;
		}
		refreshWhereLabel();
	}

	void FileTemplateInitWidget_AStoryX::onCreateButtonClicked() {
		Visindigo::Utility::JsonConfig* config = d->ConfigWidget->getConfig();
		QString completePath = config->getString("File.Path") + "/" +
			Visindigo::Utility::FileUtility::toLegelFileName(config->getString("File.Name")) + ".astory";
		QFile file(completePath);
		bool ok = !file.exists();
		if (ok) {
			QStringList fileContent;
			if (config->getString("File.Version") == "2.05.22.1A" || config->getString("File.Version") == "uptodate") {
				fileContent = initFileV3();
			}
			else {
				QMessageBox msgBox;
				msgBox.setWindowTitle(VITR("ASERStudio::fileProvider.version.unsupported.title"));
				msgBox.setText(VITR("ASERStudio::fileProvider.version.unsupported.text").arg(config->getString("File.Version")));
				msgBox.setStandardButtons(QMessageBox::Ok);
				int ret = msgBox.exec();
				return;
			}
			Visindigo::Utility::FileUtility::saveLines(completePath, fileContent);
			emit filePrepared(completePath);
			close();
		}
		else {
			QMessageBox msgBox;
			msgBox.setWindowTitle(VITR("ASERStudio::fileProvider.failed.title"));
			msgBox.setText("ASERStudio::fileProvider.failed.text");
			msgBox.setStandardButtons(QMessageBox::Ok);
			int ret = msgBox.exec();
		}
	}

	QStringList FileTemplateInitWidget_AStoryX::initFileV3() {
		QStringList fileContent = {
			"New AStory File",
			"------",
			"讲述人\t讲述内容",
			"// 此文件由ASE Dev Tool 创建，请注意不要误删除文件头信息。"
		};
		return fileContent;
	}

	void FileTemplateInitWidget_AStoryX::refreshWhereLabel() {
		QString completePath = d->FilePath + "/" +
			Visindigo::Utility::FileUtility::toLegelFileName(d->FileName) + ".astory";
		if (!Visindigo::Utility::FileUtility::isDirExist(completePath)) {
			d->WhereLabel->setText(VITR("ASERStudio::fileProvider.window.where").arg(completePath));
			d->CreateButton->setEnabled(true);
		}
		else {
			d->WhereLabel->setText(VITR("ASERStudio::fileProvider.window.exists").arg(completePath));
			d->CreateButton->setEnabled(false);
		}
	}
	FileTemplate_AStoryX::FileTemplate_AStoryX(YSSCore::Editor::EditorPlugin* plugin) :
		FileTemplateProvider("ASE AStory File Template Provider", "FileTemplate_AStoryX", plugin)
	{
		setTemplateIconPath(":/resource/cn.yxgeneral.aserstudio/icon/ASEA_Dark.png");
		setTemplateID("ASEAStoryXFile");
		setTemplateName(VITR("ASERStudio::fileProvider.astory.name"));
		setTemplateDescription(VITR("ASERStudio::fileProvider.astory.description"));
		setTemplateTags(QStringList({
				VITR("ASERStudio::fileProvider.astory.tags.ase"),
				VITR("ASERStudio::fileProvider.astory.tags.astory"),
				VITR("ASERStudio::fileProvider.astory.tags.engine-native"),
				VITR("ASERStudio::fileProvider.astory.tags.script"),
				VITR("ASERStudio::fileProvider.astory.tags.arknights")
			}));
		// Constructor implementation
	}

	FileTemplate_AStoryX::~FileTemplate_AStoryX() {
		// Destructor implementation (if needed)
	}

	YSSCore::Editor::FileTemplateInitWidget* FileTemplate_AStoryX::fileInitWidget(const QString& initPath) {
		return new FileTemplateInitWidget_AStoryX(initPath);
	}
}