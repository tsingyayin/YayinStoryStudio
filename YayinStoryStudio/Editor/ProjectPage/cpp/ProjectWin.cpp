#include "../ProjectWin.h"
#include "../ProjectInfoWidget.h"
#include <General/Version.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include "../../GlobalValue.h"
#include <Utility/JsonConfig.h>
#include <Editor/ThemeManager.h>
#include <General/TranslationHost.h>
#include <Utility/FileUtility.h>
#include <General/YSSProject.h>
#include <algorithm>
#include <Widgets/MultiButton.h>
#include <QtWidgets/qscrollbar.h>

namespace YSS::ProjectPage {
	ProjectWin::ProjectWin() :QFrame() {
		Config = new YSSCore::Utility::JsonConfig();
		QString configAll = YSSCore::Utility::FileUtility::readAll("./resource/config/project.json");
		Config->parse(configAll);
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		
		TitleLabel = new QLabel(this);
		TitleLabel->setText("  Yayin Story Studio " + YSSCore::General::Version::getYSSVersion().toString());
		TitleLabel->setObjectName("TitleLabel");
		TitleLabel->setFixedHeight(80);
		HistoryProjectArea = new QScrollArea(this);
		HistoryProjectWidget = new QWidget(HistoryProjectArea);
		HistoryProjectLayout = new QVBoxLayout(HistoryProjectWidget);
		HistoryProjectLayout->setContentsMargins(0, 0, 0, 0);
		HistoryProjectLayout->setSpacing(0);
		HistoryProjectWidget->setLayout(HistoryProjectLayout);
		HistoryProjectArea->setWidget(HistoryProjectWidget);
		HistoryProjectArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		HistoryProjectArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		NewsWidget = new QWidget(this);
		OptionWidget = new QWidget(this);
		CreateProjectButton = new QPushButton(OptionWidget);
		CreateProjectButton->setText(YSSTR("YSS::project.createNewProject"));
		OpenFolderButton = new QPushButton(OptionWidget);
		OpenFolderButton->setText(YSSTR("YSS::project.openFolder"));
		CloneGitButton = new QPushButton(OptionWidget);
		CloneGitButton->setText(YSSTR("YSS::project.cloneFromGit"));
		ButtonLayout = new QVBoxLayout(OptionWidget);
		OptionWidget->setLayout(ButtonLayout);
		ButtonLayout->addWidget(CreateProjectButton);
		ButtonLayout->addWidget(OpenFolderButton);
		ButtonLayout->addWidget(CloneGitButton);
		InfoWidget = new ProjectInfoWidget(this);
		Layout = new QGridLayout(this);

		this->setLayout(Layout);
		Layout->addWidget(TitleLabel, 0, 0, 1, 2);
		Layout->addWidget(HistoryProjectArea, 1, 0, 1, 1);
		Layout->addWidget(NewsWidget, 1, 1, 1, 1);
		Layout->addWidget(InfoWidget, 2, 0, 1, 1);
		Layout->addWidget(OptionWidget, 2, 1, 1, 1);
		Layout->setColumnStretch(0, 3);
		Layout->setColumnStretch(1, 1);
		Layout->setSpacing(0);
		Layout->setContentsMargins(0, 0, 0, 0);
		this->setStyleSheet(YSSTM->getStyleSheet("ProjectWin", this));
		
		loadProject();

		int width = GlobalValue::getConfig()->getInt("Window.Project.Width");
		int height = GlobalValue::getConfig()->getInt("Window.Project.Height");
		this->setMinimumSize(800, 600);
		this->resize(width, height);
		this->setWindowTitle("Welcome");
		if (GlobalValue::getConfig()->getBool("Window.Project.Maximized")) {
			this->showMaximized();
		}
	}

	void ProjectWin::closeEvent(QCloseEvent* event){
		YSSCore::Utility::JsonConfig* config = GlobalValue::getConfig();
		if (this->isMaximized()) {
			config->setBool("Window.Project.Maximized", true);
		}
		else {
			config->setInt("Window.Project.Width", this->width());
			config->setInt("Window.Project.Height", this->height());
			config->setBool("Window.Project.Maximized", false);
		}
	}

	void ProjectWin::resizeEvent(QResizeEvent* event) {
		HistoryProjectWidget->setFixedWidth(HistoryProjectArea->width() - HistoryProjectArea->verticalScrollBar()->width());
	}
	void ProjectWin::loadProject() {
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			delete project;
			project = nullptr;
		}
		HistoryProjectList.clear();
		for (QString key : Config->keys("Project")) {
			QString projectPath = Config->getString("Project." + key);
			YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
			bool ok = project->loadProject(projectPath);
			if (!ok) {
				delete project;
				continue;
			}
			else {
				HistoryProjectList.append(project);
			}
		}
		std::sort(HistoryProjectList.begin(), HistoryProjectList.end(), [](YSSCore::General::YSSProject* a, YSSCore::General::YSSProject* b) {
			return a->getProjectLastModifyTime() > b->getProjectLastModifyTime();
			});
		for (YSSCore::Widgets::MultiButton* label : HistoryProjectLabelList) {
			label->deleteLater();
			label = nullptr;
		}
		HistoryProjectLabelList.clear();
		for (QLabel* label : HistoryProjectTimeLabelList) {
			label->deleteLater();
			label = nullptr;
		}
		HistoryProjectTimeLabelList.clear();
		HistoryProjectMap.clear();
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			YSSCore::Widgets::MultiButton* label = new YSSCore::Widgets::MultiButton(HistoryProjectWidget);
			label->setObjectName("HistoryProject");
			label->setNormalStyleSheet(YSSTM->getStyleSheet("ProjectWin.HistoryProject.Normal"));
			label->setHoverStyleSheet(YSSTM->getStyleSheet("ProjectWin.HistoryProject.Hover"));
			label->setPressedStyleSheet(YSSTM->getStyleSheet("ProjectWin.HistoryProject.Pressed"));
			label->setTitle(project->getProjectName());
			QString lastModifyTime = project->getProjectLastModifyTime().toString("yyyy-MM-dd hh:mm:ss");
			label->setDescription(lastModifyTime+"\t"+ project->getProjectFolder());
			HistoryProjectLabelList.append(label);
			HistoryProjectLayout->addWidget(label);
			label->setSpacing(0);
			label->setContentsMargins(5, 5, 5, 5);
			//label->setFixedHeight(50);
			label->show();
			HistoryProjectMap[label] = project;
			connect(label, &YSSCore::Widgets::MultiButton::clicked, this, &ProjectWin::onProjectSelected);
		}
		HistoryProjectWidget->setFixedHeight(HistoryProjectLabelList.length() * 70);
	}
	void ProjectWin::onProjectSelected() {
		YSSCore::Widgets::MultiButton* label = qobject_cast<YSSCore::Widgets::MultiButton*>(sender());
		if (label) {
			YSSCore::General::YSSProject* project = HistoryProjectMap[label];
			if (project) {
				InfoWidget->showProject(project);
			}
		}
	}
}