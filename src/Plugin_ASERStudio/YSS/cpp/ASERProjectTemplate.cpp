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
#include <QtCore/qmap.h>
#include "YSS/DS_AStoryXDebugger.h"
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
		//d->Layout->setContentsMargins(0, 0, 0, 0);
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

	void ProjectTemplateInitWidget_AStoryX::resizeEvent(QResizeEvent* event) {
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
			project.setProjectDebugServerID(ASERStudio_AStoryXDebuggerID);
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
		Visindigo::Utility::FileUtility::copyFile(":/resource/cn.yxgeneral.aserstudio/template/3.0/BaseRule.json", projectFolder + "/Rules/BaseRule.json", true);
		Visindigo::Utility::FileUtility::copyFile(":/resource/cn.yxgeneral.aserstudio/template/3.0/AdvanceRule.json", projectFolder + "/Rules/AdvanceRule.json", true);
		Visindigo::Utility::FileUtility::copyFile(":/resource/cn.yxgeneral.aserstudio/template/3.0/main.astoryx", projectFolder + "/Stories/main.astoryx", true);
		Visindigo::Utility::FileUtility::copyFile(":/resource/cn.yxgeneral.aserstudio/template/3.0/tianyu_0.png", projectFolder + "/Resources/Char_Picture/tianyu/0.png", true);
		Visindigo::Utility::FileUtility::copyFile(":/resource/cn.yxgeneral.aserstudio/template/3.0/tianyu_1.png", projectFolder + "/Resources/Char_Picture/tianyu/1.png", true);
		Visindigo::Utility::FileUtility::copyFile(":/resource/cn.yxgeneral.aserstudio/template/3.0/classic.png", projectFolder + "/Resources/Background/bg.png", true);
		Visindigo::Utility::FileUtility::copyFile(":/resource/cn.yxgeneral.aserstudio/template/3.0/classic.png", projectFolder + "/cover.png", true);
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
		ProjectTemplateProvider("ASE AStory Project Template Provider", "ProjectTemplate_AStoryX", plugin) {
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

	class ProjectTemplateInitWidget_AStoryX_3_6_7Private {
		friend class ProjectTemplateInitWidget_AStoryX_3_6_7;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget;
		QLabel* WhereLabel;
		QPushButton* CreateButton;
		QVBoxLayout* Layout;
		QHBoxLayout* ButtonLayout;
		QString ProjectPath;
		QString ProjectName;
	};
	ProjectTemplateInitWidget_AStoryX_3_6_7::ProjectTemplateInitWidget_AStoryX_3_6_7(QWidget* parent)
		: YSSCore::Editor::ProjectTemplateInitWidget(parent)
	{
		d = new ProjectTemplateInitWidget_AStoryX_3_6_7Private();
		this->setMinimumWidth(800);
		this->setWindowTitle(VITR("ASERStudio::provider.window.title"));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget(this);
		d->ConfigWidget->loadCWJson(Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.aserstudio/configWidget/PTP_3_6_7.json"));
		d->Layout = new QVBoxLayout(this);
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
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::lineEditTextChanged, this, &ProjectTemplateInitWidget_AStoryX_3_6_7::onLineEditTextChanged);
		connect(d->CreateButton, &QPushButton::clicked, this, &ProjectTemplateInitWidget_AStoryX_3_6_7::onCreateButtonClicked);
		refreshWhereLabel();
	}

	ProjectTemplateInitWidget_AStoryX_3_6_7::~ProjectTemplateInitWidget_AStoryX_3_6_7() {
		delete d;
	}

	void ProjectTemplateInitWidget_AStoryX_3_6_7::resizeEvent(QResizeEvent* event) {
		YSSCore::Editor::ProjectTemplateInitWidget::resizeEvent(event);
	}

	void ProjectTemplateInitWidget_AStoryX_3_6_7::onLineEditTextChanged(const QString& node, const QString& str) {
		if (node == "Project.Path") {
			d->ProjectPath = str;
		}
		else if (node == "Project.Name") {
			d->ProjectName = str;
		}
		refreshWhereLabel();
	}

	void ProjectTemplateInitWidget_AStoryX_3_6_7::onCreateButtonClicked() {
		Visindigo::Utility::JsonConfig* config = d->ConfigWidget->getConfig();
		YSSCore::General::YSSProject project;
		QString completePath = config->getString("Project.Path") + "/" + Visindigo::Utility::FileUtility::toLegelFileName(config->getString("Project.Name"));
		bool ok = project.initProject(completePath, config->getString("Project.Name"));
		if (ok) {
			project.setProjectIconPath("cover.png");
			project.setProjectDebugServerID(ASERStudio_AStoryXDebuggerID);
			project.saveProject();
			initProjectV3(&project);
			emit projectPrepared(project.getProjectPath());
			close();
		}
		else {
			QMessageBox msgBox;
			msgBox.setWindowTitle(VITR("ASERStudio::provider.failed.title"));
			msgBox.setText(VITR("ASERStudio::provider.failed.text"));
			msgBox.setStandardButtons(QMessageBox::Ok);
			int ret = msgBox.exec();
		}
	}

	void ProjectTemplateInitWidget_AStoryX_3_6_7::initProjectV3(YSSCore::General::YSSProject* project) {
		Visindigo::Utility::JsonConfig projectConfig = project->getProjectConfigForPlugin("cn.yxgeneral.aserstudio");
		projectConfig.setString("ASERVersion", "3.6.7");
		project->saveProjectConfigForPlugin("cn.yxgeneral.aserstudio", projectConfig);
		QString projectFolder = project->getProjectFolder();
		const QString templatePath = ":/resource/cn.yxgeneral.aserstudio/template/3.6.7";
		const QMap<QString, QString> fileMap = {
			{ "Configs/override_aliases.json", "Configs/override_aliases.json" },
			{ "Configs/override_userConfigs.json", "Configs/override_userConfigs.json" },
			{ "Resources/Background/bg_amb2026.png", "Resources/Background/bg_amb2026.png" },
			{ "Resources/Char_Picture/mon3tr(amb2026)/mon3tr(amb2026)_config.json", "Resources/Char_Picture/mon3tr(amb2026)/mon3tr(amb2026)_config.json" },
			{ "Resources/Char_Picture/mon3tr(amb2026)/normal.png", "Resources/Char_Picture/mon3tr(amb2026)/normal.png" },
			{ "Resources/Char_Spine/Amiya(furnace_finale)/Amiya(furnace_finale).atlas", "Resources/Char_Spine/阿米娅(furnace_finale)/阿米娅(furnace_finale).atlas" },
			{ "Resources/Char_Spine/Amiya(furnace_finale)/Amiya(furnace_finale).png", "Resources/Char_Spine/阿米娅(furnace_finale)/阿米娅(furnace_finale).png" },
			{ "Resources/Char_Spine/Amiya(furnace_finale)/Amiya(furnace_finale).skel", "Resources/Char_Spine/阿米娅(furnace_finale)/阿米娅(furnace_finale).skel" },
			{ "Resources/Char_Spine/Amiya(furnace_finale)/Amiya(furnace_finale)_config.json", "Resources/Char_Spine/阿米娅(furnace_finale)/阿米娅(furnace_finale)_config.json" },
			{ "Resources/Music/immutable_ins.wav", "Resources/Music/immutable_ins.wav" },
			{ "Resources/SoundEffect/MagneticBurst.wav", "Resources/SoundEffect/磁爆.wav" },
			{ "Resources/VisualEffect/manifest.json", "Resources/VisualEffect/manifest.json" },
			{ "Resources/VisualEffect/prefab/furnace_finale.ab", "Resources/VisualEffect/prefab/furnace_finale.ab" },
			{ "Resources/VisualEffect/shader/shader.ab", "Resources/VisualEffect/shader/shader.ab" },
			{ "Resources/VisualEffect/texture/noise.ab", "Resources/VisualEffect/texture/noise.ab" },
			{ "Resources/VisualEffect/texture/ring.ab", "Resources/VisualEffect/texture/ring.ab" },
			{ "Rules/AdvanceRule.json", "Rules/AdvanceRule.json" },
			{ "Rules/BaseRule.json", "Rules/BaseRule.json" },
			{ "Stories/00_BasicExample_1.astoryx", "Stories/00_基础示例上.astoryx" },
			{ "Stories/01_BasicExample_2.astoryx", "Stories/01_基础示例下.astoryx" },
			{ "Stories/02_BasicExample_3.astoryx", "Stories/02_基础？？？.astoryx" }
		};
		for (const QString& key : fileMap.keys()) {
			Visindigo::Utility::FileUtility::copyFile(templatePath + "/" + key, projectFolder + "/" + fileMap.value(key), true, true);
		}
		Visindigo::Utility::FileUtility::copyFile(templatePath + "/Resources/Background/bg_amb2026.png", projectFolder + "/cover.png", true);
		project->addEditorOpenedFile(projectFolder + "/Stories/00_基础示例上.astoryx");
		project->addEditorOpenedFile(projectFolder + "/Stories/01_基础示例下.astoryx");
		project->addEditorOpenedFile(projectFolder + "/Stories/02_基础？？？.astoryx");
		project->setFocusedFile(projectFolder + "/Stories/00_基础示例上.astoryx");
		project->saveProject();
	}

	void ProjectTemplateInitWidget_AStoryX_3_6_7::refreshWhereLabel() {
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

	ProjectTemplate_AStoryX_3_6_7::ProjectTemplate_AStoryX_3_6_7(YSSCore::Editor::EditorPlugin* plugin) :
		ProjectTemplateProvider("ASE AStory 3.6.7 Project Template Provider", "ProjectTemplate_AStoryX_3_6_7", plugin) {
		setTemplateIconPath(":/resource/cn.yxgeneral.aserstudio/icon/ASEA_Dark.png");
		setTemplateID("ASEAStoryXProject367");
		setTemplateName(VITR("ASERStudio::provider.astory367.name"));
		setTemplateDescription(VITR("ASERStudio::provider.astory367.description"));
		setTemplateTags(QStringList({
				VITR("ASERStudio::provider.astory.tags.ase"),
				VITR("ASERStudio::provider.astory.tags.astory"),
				VITR("ASERStudio::provider.astory.tags.engine-native"),
				VITR("ASERStudio::provider.astory.tags.script"),
				VITR("ASERStudio::provider.astory.tags.arknights")
			}));
	}

	ProjectTemplate_AStoryX_3_6_7::~ProjectTemplate_AStoryX_3_6_7() {
	}

	YSSCore::Editor::ProjectTemplateInitWidget* ProjectTemplate_AStoryX_3_6_7::projectInitWidget() {
		return new ProjectTemplateInitWidget_AStoryX_3_6_7();
	}
}