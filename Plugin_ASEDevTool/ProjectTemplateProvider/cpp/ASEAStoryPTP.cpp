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
	: YSSCore::Editor::ProjectTemplateInitWidget(parent)
{
	this->setMinimumWidth(800);
	this->setWindowTitle(YSSTR("ASEDevTool::provider.window.title"));
	ConfigWidget = new YSSCore::Widgets::ConfigWidget(this);
	ConfigWidget->loadCWJson(YSSCore::Utility::FileUtility::readAll( ":/plugin/compiled/ASEDevTool/configWidget/PTP.json"));
	Layout = new QVBoxLayout(this);
	Layout->setContentsMargins(0, 0, 0, 0);
	Layout->addWidget(ConfigWidget);
	ButtonLayout = new QHBoxLayout(this);
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
	connect(ConfigWidget, &YSSCore::Widgets::ConfigWidget::lineEditTextChanged, this, &ASEAStoryPTIW::onLineEditTextChanged);
	connect(CreateButton, &QPushButton::clicked, this, &ASEAStoryPTIW::onCreateButtonClicked);
	refreshWhereLabel();
	// Constructor implementation
}
void ASEAStoryPTIW::resizeEvent(QResizeEvent* event)
{
	YSSCore::Editor::ProjectTemplateInitWidget::resizeEvent(event);
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
	YSSCore::Utility::JsonConfig* config = ConfigWidget->getConfig();
	YSSCore::General::YSSProject project;
	QString completePath = config->getString("Project.Path") + "/" + YSSCore::Utility::FileUtility::toLegelFileName(config->getString("Project.Name"));
	bool ok = project.initProject(completePath, config->getString("Project.Name"));
	if (ok) {
		project.getProjectConfig()->setString("Project.DebugServerID", "ASEDevTool_ASE");
		project.saveProject();
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
void ASEAStoryPTIW::refreshWhereLabel() {
	QString completePath = ProjectPath + "/" + YSSCore::Utility::FileUtility::toLegelFileName(ProjectName);
	if (!YSSCore::Utility::FileUtility::isDirExist(completePath)) {
		WhereLabel->setText(YSSTR("ASEDevTool::provider.window.where").arg(completePath));
		CreateButton->setEnabled(true);
	}
	else {
		WhereLabel->setText(YSSTR("ASEDevTool::provider.window.exists").arg(completePath));
		CreateButton->setEnabled(false);
	}
	
}
ASEAStoryPTP::ASEAStoryPTP(YSSCore::Editor::EditorPlugin* plugin):ProjectTemplateProvider(plugin)
{
	setTemplateIcon(QIcon(":/plugin/compiled/ASEDevTool/icon/provider.png"));
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

YSSCore::Editor::ProjectTemplateInitWidget* ASEAStoryPTP::projectInitWidget(){
	return new ASEAStoryPTIW();
}