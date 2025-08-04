#include "../ASEAStoryFTP.h"
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

ASEAStoryFTIW::ASEAStoryFTIW(const QString& initFolder, QWidget* parent)
	: Visindigo::Editor::FileTemplateInitWidget(initFolder, parent)
{
	this->setMinimumWidth(800);
	this->setWindowTitle(YSSTR("ASEDevTool::fileProvider.window.title"));
	ConfigWidget = new Visindigo::Widgets::ConfigWidget(this);
	ConfigWidget->loadCWJson(Visindigo::Utility::FileUtility::readAll(":/plugin/compiled/ASEDevTool/configWidget/FTP.json"));
	ConfigWidget->setLineEditText("File.Path", getInitFolder());
	Layout = new QVBoxLayout(this);
	Layout->setContentsMargins(0, 0, 0, 0);
	Layout->addWidget(ConfigWidget);
	ButtonLayout = new QHBoxLayout();
	ButtonLayout->setContentsMargins(10, 0, 10, 0);
	Layout->addLayout(ButtonLayout);
	WhereLabel = new QLabel(this);
	WhereLabel->setWordWrap(true);
	ButtonLayout->addWidget(WhereLabel);
	CreateButton = new QPushButton(YSSTR("ASEDevTool::provider.window.create"), this);
	CreateButton->setObjectName("CreateButton");
	CreateButton->setMinimumWidth(120);
	CreateButton->setMaximumWidth(160);
	ButtonLayout->addWidget(CreateButton);
	FilePath = ConfigWidget->getConfig()->getString("File.Path");
	FileName = ConfigWidget->getConfig()->getString("File.Name");
	connect(ConfigWidget, &Visindigo::Widgets::ConfigWidget::lineEditTextChanged, this, &ASEAStoryFTIW::onLineEditTextChanged);
	connect(CreateButton, &QPushButton::clicked, this, &ASEAStoryFTIW::onCreateButtonClicked);
	refreshWhereLabel();
}

void ASEAStoryFTIW::resizeEvent(QResizeEvent* event) {
	Visindigo::Editor::FileTemplateInitWidget::resizeEvent(event);
}

void ASEAStoryFTIW::onLineEditTextChanged(const QString& node, const QString& text) {
	if (node == "File.Path") {
		FilePath = text;
	} else if (node == "File.Name") {
		FileName = text;
	}
	refreshWhereLabel();
}

void ASEAStoryFTIW::onCreateButtonClicked() {
	Visindigo::Utility::JsonConfig* config = ConfigWidget->getConfig();
	QString completePath = config->getString("File.Path") + "/" +
		Visindigo::Utility::FileUtility::toLegelFileName(config->getString("File.Name")) + ".astory";
	QFile file(completePath);
	bool ok = !file.exists();
	if (ok) {
		QStringList fileContent;
		if (config->getString("File.Version") == "2.05.22.1A" || config->getString("File.Version") == "uptodate") {
			fileContent = initFileV2_05_22_1A();
		}
		else {
			QMessageBox msgBox;
			msgBox.setWindowTitle(YSSTR("ASEDevTool::fileProvider.version.unsupported.title"));
			msgBox.setText(YSSTR("ASEDevTool::fileProvider.version.unsupported.text").arg(config->getString("File.Version")));
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
		msgBox.setWindowTitle(YSSTR("ASEDevTool::fileProvider.failed.title"));
		msgBox.setText("ASEDevTool::fileProvider.failed.text");
		msgBox.setStandardButtons(QMessageBox::Ok);
		int ret = msgBox.exec();
	}
}

QStringList ASEAStoryFTIW::initFileV2_05_22_1A() {
	QStringList fileContent = {
		"New AStory File",
		"------",
		"讲述人\t讲述内容",
		"// 此文件由ASE Dev Tool 创建，请注意不要误删除文件头信息。"
	};
	return fileContent;
}

void ASEAStoryFTIW::refreshWhereLabel() {
	QString completePath = FilePath + "/" +
		Visindigo::Utility::FileUtility::toLegelFileName(FileName) + ".astory";
	if (!Visindigo::Utility::FileUtility::isDirExist(completePath)) {
		WhereLabel->setText(YSSTR("ASEDevTool::fileProvider.window.where").arg(completePath));
		CreateButton->setEnabled(true);
	}
	else {
		WhereLabel->setText(YSSTR("ASEDevTool::fileProvider.window.exists").arg(completePath));
		CreateButton->setEnabled(false);
	}
}
ASEAStoryFTP::ASEAStoryFTP(Visindigo::Editor::EditorPlugin* plugin) :
	FileTemplateProvider("ASE AStory File Template Provider", "ASEAStoryFTP", plugin)
{
	setTemplateIconPath(":/plugin/compiled/ASEDevTool/icon/ASEA_Dark.png");
	setTemplateID("ASEAStoryFile");
	setTemplateName(YSSTR("ASEDevTool::fileProvider.astory.name"));
	setTemplateDescription(YSSTR("ASEDevTool::fileProvider.astory.description"));
	setTemplateTags(QStringList({
			YSSTR("ASEDevTool::fileProvider.astory.tags.ase"),
			YSSTR("ASEDevTool::fileProvider.astory.tags.astory"),
			YSSTR("ASEDevTool::fileProvider.astory.tags.engine-native"),
			YSSTR("ASEDevTool::fileProvider.astory.tags.script"),
			YSSTR("ASEDevTool::fileProvider.astory.tags.arknights")
		}));
	// Constructor implementation
}

Visindigo::Editor::FileTemplateInitWidget* ASEAStoryFTP::fileInitWidget(const QString& initPath) {
	return new ASEAStoryFTIW(initPath);
}