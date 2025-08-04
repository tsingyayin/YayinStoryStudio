#include "../ASEAStoryPTP.h"
#include <QtWidgets/qboxlayout.h>
#include <General/TranslationHost.h>
#include <Widgets/ConfigWidget.h>
#include <Utility/FileUtility.h>
#include <QtWidgets/qpushbutton.h>
#include <Utility/JsonConfig.h>
#include <QtWidgets/qlabel.h>
#include <General/YSSProject.h>
#include <QtWidgets/qmessagebox.h>
ASEAStoryPTIW::ASEAStoryPTIW(QWidget* parent)
	: Visindigo::Editor::ProjectTemplateInitWidget(parent)
{
	this->setMinimumWidth(800);
	this->setWindowTitle(YSSTR("ASEDevTool::provider.window.title"));
	ConfigWidget = new Visindigo::Widgets::ConfigWidget(this);
	ConfigWidget->loadCWJson(Visindigo::Utility::FileUtility::readAll(":/plugin/compiled/ASEDevTool/configWidget/PTP.json"));
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
	ProjectPath = ConfigWidget->getConfig()->getString("Project.Path");
	ProjectName = ConfigWidget->getConfig()->getString("Project.Name");
	connect(ConfigWidget, &Visindigo::Widgets::ConfigWidget::lineEditTextChanged, this, &ASEAStoryPTIW::onLineEditTextChanged);
	connect(CreateButton, &QPushButton::clicked, this, &ASEAStoryPTIW::onCreateButtonClicked);
	refreshWhereLabel();
	// Constructor implementation
}
void ASEAStoryPTIW::resizeEvent(QResizeEvent* event)
{
	Visindigo::Editor::ProjectTemplateInitWidget::resizeEvent(event);
}
void ASEAStoryPTIW::onLineEditTextChanged(const QString& node, const QString& str) {
	if (node == "Project.Path") {
		ProjectPath = str;
	}
	else if (node == "Project.Name") {
		ProjectName = str;
	}
	refreshWhereLabel();
}
void ASEAStoryPTIW::onCreateButtonClicked() {
	Visindigo::Utility::JsonConfig* config = ConfigWidget->getConfig();
	Visindigo::General::YSSProject project;
	QString completePath = config->getString("Project.Path") + "/" + Visindigo::Utility::FileUtility::toLegelFileName(config->getString("Project.Name"));
	bool ok = project.initProject(completePath, config->getString("Project.Name"));
	if (ok) {
		project.getProjectConfig()->setString("Project.DebugServerID", "ASEDevTool_ASE");
		project.saveProject();
		QString projectFolder = project.getProjectFolder();
		if (config->getString("Project.Version") == "2.05.22.1A" || config->getString("Project.Version") == "uptodate") {
			initResourceV2_05_22_1A(&project);
		}
		else {
			QMessageBox msgBox;
			msgBox.setWindowTitle(YSSTR("ASEDevTool::provider.version.unsupported.title"));
			msgBox.setText(YSSTR("ASEDevTool::provider.version.unsupported.text").arg(config->getString("Project.Version")));
			msgBox.setStandardButtons(QMessageBox::Ok);
			int ret = msgBox.exec();
			return;
		}
		emit projectPrepared(project.getProjectPath());
		close();
	}
	else {
		QMessageBox msgBox;
		msgBox.setWindowTitle(YSSTR("ASEDevTool::provider.failed.title"));
		msgBox.setText("ASEDevTool::provider.failed.text");
		msgBox.setStandardButtons(QMessageBox::Ok);
		int ret = msgBox.exec();
	}
}
void ASEAStoryPTIW::initResourceV2_05_22_1A(Visindigo::General::YSSProject* project) {
	project->getProjectConfig()->setString("ASEDevTool.ASEVersion", "2.05.22.1A");
	QStringList folders = {
			"/Resources/Audio",
			"/Resources/BGP",
			"/Resources/Char",
			"/Resources/Char_Spine",
			"/Rules/CharNameConnect",
			"/Rules/StoryExplainer",
			"/Stories"
	};
	QString projectFolder = project->getProjectFolder();
	for (const QString& folder : folders) {
		Visindigo::Utility::FileUtility::createDir(projectFolder + folder);
	}
	QString Base_aschar = Visindigo::Utility::FileUtility::readAll(":/plugin/compiled/ASEDevTool/template/2.05.22.1A/Base.aschar");
	QString BaseRule_asrule = Visindigo::Utility::FileUtility::readAll(":/plugin/compiled/ASEDevTool/template/2.05.22.1A/BaseRule.asrule");
	QString main_astory = Visindigo::Utility::FileUtility::readAll(":/plugin/compiled/ASEDevTool/template/2.05.22.1A/main.astory");
	main_astory = main_astory.replace("$(ProjectName)", project->getProjectName());
	Visindigo::Utility::FileUtility::saveAll(projectFolder + "/Rules/CharNameConnect/Base.aschar", Base_aschar);
	Visindigo::Utility::FileUtility::saveAll(projectFolder + "/Rules/StoryExplainer/BaseRule.asrule", BaseRule_asrule);
	Visindigo::Utility::FileUtility::saveAll(projectFolder + "/Stories/main.astory", main_astory);
	project->addEditorOpenedFile(projectFolder + "/Stories/main.astory");
	project->setFocusedFile(projectFolder + "/Stories/main.astory");
	project->saveProject();
}
void ASEAStoryPTIW::refreshWhereLabel() {
	QString completePath = ProjectPath + "/" + Visindigo::Utility::FileUtility::toLegelFileName(ProjectName);
	if (!Visindigo::Utility::FileUtility::isDirExist(completePath)) {
		WhereLabel->setText(YSSTR("ASEDevTool::provider.window.where").arg(completePath));
		CreateButton->setEnabled(true);
	}
	else {
		WhereLabel->setText(YSSTR("ASEDevTool::provider.window.exists").arg(completePath));
		CreateButton->setEnabled(false);
	}
}
ASEAStoryPTP::ASEAStoryPTP(Visindigo::Editor::EditorPlugin* plugin) :
	ProjectTemplateProvider("ASE AStory Project Template Provider", "ASEAStoryPTP", plugin)
{
	setTemplateIconPath(":/plugin/compiled/ASEDevTool/icon/ASEA_Dark.png");
	setTemplateID("ASEAStoryProject");
	setTemplateName(YSSTR("ASEDevTool::provider.astory.name"));
	setTemplateDescription(YSSTR("ASEDevTool::provider.astory.description"));
	setTemplateTags(QStringList({
			YSSTR("ASEDevTool::provider.astory.tags.ase"),
			YSSTR("ASEDevTool::provider.astory.tags.astory"),
			YSSTR("ASEDevTool::provider.astory.tags.engine-native"),
			YSSTR("ASEDevTool::provider.astory.tags.script"),
			YSSTR("ASEDevTool::provider.astory.tags.arknights")
		}));
	// Constructor implementation
}

Visindigo::Editor::ProjectTemplateInitWidget* ASEAStoryPTP::projectInitWidget() {
	return new ASEAStoryPTIW();
}