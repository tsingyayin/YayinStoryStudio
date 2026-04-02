#include "YSS/ASERProjectTemplate.h"
#include <QtWidgets/qboxlayout.h>
#include <General/TranslationHost.h>
#include <Widgets/ConfigWidget.h>
#include <Utility/FileUtility.h>
#include <QtWidgets/qpushbutton.h>
#include <Utility/JsonConfig.h>
#include <QtWidgets/qlabel.h>
#include <General/YSSProject.h>
#include <QtWidgets/qmessagebox.h>
#include <Editor/EditorPlugin.h>
#include <QtCore/qfile.h>

namespace ASERStudio::YSS {
	class ProjectTemplateInitWidget_AStoryXPrivate {
		friend class ProjectTemplateInitWidget_AStoryX;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget;
		QLabel* WhereLabel;
		QPushButton* CreateButton;
		QVBoxLayout* Layout;
		QHBoxLayout* ButtonLayout;
		QString ProjectPath;
		QString ProjectName;
	};
	ProjectTemplateInitWidget_AStoryX::ProjectTemplateInitWidget_AStoryX(QWidget* parent)
		: YSSCore::Editor::ProjectTemplateInitWidget(parent)
	{
		d = new ProjectTemplateInitWidget_AStoryXPrivate();
		this->setMinimumWidth(800);
		this->setWindowTitle(VITR("ASERStudio::provider.window.title"));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget(this);
		d->ConfigWidget->loadCWJson(Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.aserstudio/configWidget/PTP.json"));
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
		d->ProjectPath = d->ConfigWidget->getConfig()->getString("Project.Path");
		d->ProjectName = d->ConfigWidget->getConfig()->getString("Project.Name");
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::lineEditTextChanged, this, &ProjectTemplateInitWidget_AStoryX::onLineEditTextChanged);
		connect(d->CreateButton, &QPushButton::clicked, this, &ProjectTemplateInitWidget_AStoryX::onCreateButtonClicked);
		refreshWhereLabel();
		// Constructor implementation
	}

	ProjectTemplateInitWidget_AStoryX::~ProjectTemplateInitWidget_AStoryX() {
		delete d;
	}

	void ProjectTemplateInitWidget_AStoryX::resizeEvent(QResizeEvent* event){
		YSSCore::Editor::ProjectTemplateInitWidget::resizeEvent(event);
	}

	void ProjectTemplateInitWidget_AStoryX::onLineEditTextChanged(const QString& node, const QString& str) {
		if (node == "Project.Path") {
			d->ProjectPath = str;
		}
		else if (node == "Project.Name") {
			d->ProjectName = str;
		}
		refreshWhereLabel();
	}

	void ProjectTemplateInitWidget_AStoryX::onCreateButtonClicked() {
		Visindigo::Utility::JsonConfig* config = d->ConfigWidget->getConfig();
		YSSCore::General::YSSProject project;
		QString completePath = config->getString("Project.Path") + "/" + Visindigo::Utility::FileUtility::toLegelFileName(config->getString("Project.Name"));
		bool ok = project.initProject(completePath, config->getString("Project.Name"));
		if (ok) {
			project.setProjectIconPath("cover.png");
			project.setProjectDebugServerID("cn.yxgeneral.aserstudio.astory");
			project.saveProject();
			QString projectFolder = project.getProjectFolder();
			QStringList CompGroup_2_05_22_1A = {
					"uptodate"
			};
			if (CompGroup_2_05_22_1A.contains(config->getString("Project.Version"))) {
				initProjectV3(&project);
			}
			else {
				QMessageBox msgBox;
				msgBox.setWindowTitle(VITR("ASERStudio::provider.version.unsupported.title"));
				msgBox.setText(VITR("ASERStudio::provider.version.unsupported.text").arg(config->getString("Project.Version")));
				msgBox.setStandardButtons(QMessageBox::Ok);
				int ret = msgBox.exec();
				return;
			}
			emit projectPrepared(project.getProjectPath());
			close();
		}
		else {
			QMessageBox msgBox;
			msgBox.setWindowTitle(VITR("ASERStudio::provider.failed.title"));
			msgBox.setText("ASERStudio::provider.failed.text");
			msgBox.setStandardButtons(QMessageBox::Ok);
			int ret = msgBox.exec();
		}
	}

	void ProjectTemplateInitWidget_AStoryX::initProjectV3(YSSCore::General::YSSProject* project) {
		Visindigo::Utility::JsonConfig projectConfig = project->getProjectConfigForPlugin("cn.yxgeneral.aserstudio");
		projectConfig.setString("ASERVersion", "3.0");
		project->saveProjectConfigForPlugin("cn.yxgeneral.aserstudio", projectConfig);
		QStringList folders = {
				"/Resources/Music",
				"/Resources/Background",
				"/Resources/Char_Picture/tianyu",
				"/Resources/Char_Spine",
				"/Resources/SoundEffect",
				"/Rules",
				"/Configs",
				"/Stories"
		};
		QString projectFolder = project->getProjectFolder();
		for (const QString& folder : folders) {
			Visindigo::Utility::FileUtility::createDir(projectFolder + folder);
		}
		QFile::copy(":/resource/cn.yxgeneral.aserstudio/template/3.0/BaseRule.json", projectFolder + "/Rules/BaseRule.json");
		QFile::copy(":/resource/cn.yxgeneral.aserstudio/template/3.0/AdvanceRule.json", projectFolder + "/Rules/AdvanceRule.json");
		QFile::copy(":/resource/cn.yxgeneral.aserstudio/template/3.0/main.astoryx", projectFolder + "/Stories/main.astoryx");
		QFile::copy(":/resource/cn.yxgeneral.aserstudio/template/3.0/tianyu_0.png", projectFolder + "/Resources/Char_Picture/tianyu/tianyu_0.png");
		QFile::copy(":/resource/cn.yxgeneral.aserstudio/template/3.0/tianyu_1.png", projectFolder + "/Resources/Char_Picture/tianyu/tianyu_1.png");
		QFile::copy(":/resource/cn.yxgeneral.aserstudio/template/3.0/classic.png", projectFolder + "/Resources/Background/bg.png");
		QFile::copy(":/resource/cn.yxgeneral.aserstudio/template/3.0/classic.png", projectFolder + "/cover.png");
		project->addEditorOpenedFile(projectFolder + "/Stories/main.astoryx");
		project->setFocusedFile(projectFolder + "/Stories/main.astoryx");
		project->saveProject();
	}

	void ProjectTemplateInitWidget_AStoryX::refreshWhereLabel() {
		QString completePath = d->ProjectPath + "/" + Visindigo::Utility::FileUtility::toLegelFileName(d->ProjectName);
		if (!Visindigo::Utility::FileUtility::isDirExist(completePath)) {
			d->WhereLabel->setText(VITR("ASERStudio::provider.window.where").arg(completePath));
			d->CreateButton->setEnabled(true);
		}
		else {
			d->WhereLabel->setText(VITR("ASERStudio::provider.window.exists").arg(completePath));
			d->CreateButton->setEnabled(false);
		}
	}

	ProjectTemplate_AStoryX::ProjectTemplate_AStoryX(YSSCore::Editor::EditorPlugin* plugin) :
		ProjectTemplateProvider("ASE AStory Project Template Provider", "ProjectTemplate_AStoryX", plugin){
		setTemplateIconPath(":/resource/cn.yxgeneral.aserstudio/icon/ASEA_Dark.png");
		setTemplateID("ASEAStoryXProject");
		setTemplateName(VITR("ASERStudio::provider.astory.name"));
		setTemplateDescription(VITR("ASERStudio::provider.astory.description"));
		setTemplateTags(QStringList({
				VITR("ASERStudio::provider.astory.tags.ase"),
				VITR("ASERStudio::provider.astory.tags.astory"),
				VITR("ASERStudio::provider.astory.tags.engine-native"),
				VITR("ASERStudio::provider.astory.tags.script"),
				VITR("ASERStudio::provider.astory.tags.arknights")
			}));
		// Constructor implementation
	}

	ProjectTemplate_AStoryX::~ProjectTemplate_AStoryX() {
		// Destructor implementation
	}

	YSSCore::Editor::ProjectTemplateInitWidget* ProjectTemplate_AStoryX::projectInitWidget() {
		return new ProjectTemplateInitWidget_AStoryX();
	}
}