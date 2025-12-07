#include "../ProjectWin.h"
#include "../ProjectInfoWidget.h"
#include <General/Version.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include "../../GlobalValue.h"
#include <Utility/JsonConfig.h>
#include <Widgets/ThemeManager.h>
#include <General/TranslationHost.h>
#include <Utility/FileUtility.h>
#include <General/YSSProject.h>
#include <algorithm>
#include <Widgets/MultiButton.h>
#include <QtWidgets/qscrollbar.h>
#include "../../MainEditor/MainWin.h"
#include <General/Log.h>
#include <QtWidgets/qfiledialog.h>
#include "../../NewProjectPage/NewProjectWin.h"

namespace YSS::ProjectPage {
	ProjectWin::ProjectWin() :QFrame() {
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		this->setMinimumSize(800, 600);
		this->setWindowTitle(YSSTR("YSS::project.projectManager"));
		Config = new Visindigo::Utility::JsonConfig();
		QString configAll = Visindigo::Utility::FileUtility::readAll("./resource/config/project.json");
		Config->parse(configAll);

		TitleLabel = new QLabel(this);
		TitleLabel->setText("  Yayin Story Studio " + Visindigo::General::Version::getAPIVersion().toString());
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
		CreateProjectButton->setToolTip(YSSTR("YSS::tooltips.projectWin.createNewProject"));
		OpenFolderButton = new QPushButton(OptionWidget);
		OpenFolderButton->setText(YSSTR("YSS::project.openFolder"));
		OpenFolderButton->setToolTip(YSSTR("YSS::tooltips.projectWin.openFolder"));
		CloneGitButton = new QPushButton(OptionWidget);
		CloneGitButton->setText(YSSTR("YSS::project.cloneFromGit"));
		CloneGitButton->setToolTip(YSSTR("YSS::tooltips.projectWin.cloneFromGit"));
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
		this->setStyleSheet(YSSTMSS("YSS::ProjectWin", this));

		loadProject();

		int width = GlobalValue::getConfig()->getInt("Window.Project.Width");
		int height = GlobalValue::getConfig()->getInt("Window.Project.Height");

		this->resize(width, height);
		if (GlobalValue::getConfig()->getBool("Window.Project.Maximized")) {
			this->showMaximized();
		}
		connect(CreateProjectButton, &QPushButton::clicked, this, &ProjectWin::onCreateProject);
		connect(InfoWidget, &ProjectInfoWidget::removeConfirmed, this, &ProjectWin::onProjectRemoved);
		connect(OpenFolderButton, &QPushButton::clicked, this, &ProjectWin::onOpenProjectClicked);
	}

	void ProjectWin::closeEvent(QCloseEvent* event) {
		Visindigo::Utility::FileUtility::saveAll("./resource/config/project.json", Config->toString());
		Visindigo::Utility::JsonConfig* config = GlobalValue::getConfig();
		if (this->isMaximized()) {
			config->setBool("Window.Project.Maximized", true);
		}
		else {
			config->setInt("Window.Project.Width", this->width());
			config->setInt("Window.Project.Height", this->height());
			config->setBool("Window.Project.Maximized", false);
		}
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			delete project;
			project = nullptr;
		}
		if (GlobalValue::getCurrentProject() != nullptr) {
			GlobalValue::getMainWindow()->show();
			this->deleteLater();
		}
	}

	void ProjectWin::resizeEvent(QResizeEvent* event) {
		HistoryProjectWidget->setFixedWidth(HistoryProjectArea->width() - HistoryProjectArea->verticalScrollBar()->width());
	}

	void ProjectWin::onProjectRemoved(YSSCore::General::YSSProject* project) {
		QWidget* widget = HistoryProjectLabelList[HistoryProjectList.indexOf(project)];
		if (widget != nullptr) {
			widget->hide();
			HistoryProjectLabelList.remove(HistoryProjectList.indexOf(project));
			HistoryProjectList.removeAll(project);
			HistoryProjectMap.remove(static_cast<Visindigo::Widgets::MultiButton*>(widget));
			HistoryProjectLayout->removeWidget(widget);
			widget->deleteLater();
		}
		QStringList keys = Config->keys("Project");
		for (int i = 0; i < keys.size(); i++) {
			if (Config->getString("Project." + keys[i]) == project->getProjectPath()) {
				yMessage << i;
				Config->remove("Project." + keys[i]);
				break;
			}
		}

		HistoryProjectWidget->setFixedHeight(HistoryProjectLabelList.length() * 100);
		delete project;
		project = nullptr;
	}
	void ProjectWin::onProjectSelected() {
		Visindigo::Widgets::MultiButton* label = qobject_cast<Visindigo::Widgets::MultiButton*>(sender());
		if (label) {
			YSSCore::General::YSSProject* project = HistoryProjectMap[label];
			if (project) {
				InfoWidget->showProject(project);
			}
		}
	}

	void ProjectWin::onProjectDoubleClicked() {
		Visindigo::Widgets::MultiButton* label = qobject_cast<Visindigo::Widgets::MultiButton*>(sender());
		YSSCore::General::YSSProject* project = HistoryProjectMap[label];
		HistoryProjectList.removeAll(project);
		GlobalValue::setCurrentProject(project);
		this->close();
	}

	void ProjectWin::onOpenProjectClicked() {
		onOpenProject();
	}
	void ProjectWin::onOpenProject(QString filePath) {
		if (filePath.isEmpty()) {
			filePath = QFileDialog::getOpenFileName(this, YSSTR("YSS::project.openYSSProject"), "./resource/repos", "YSS Project (*.yssp);;YSS Project (yssproj.json)");
		}
		filePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(QDir::currentPath(), filePath);
		YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
		bool ok = project->loadProject(filePath);
		yDebugF << ok;
		if (ok) {
			bool inList = false;
			for (QString key : Config->keys("Project")) {
				QString projectPath = Config->getString("Project." + key);
				if (projectPath == filePath) { inList = true; break; }
			}
			if (!inList) {
				yMessage << "Add" << filePath << "to project list.";
				Config->setString("Project." + QString::number(Config->keys("Project").size()), filePath);
			}
			GlobalValue::setCurrentProject(project);
			this->close();
		}
		else {
			delete project;
		}
	}

	void ProjectWin::onCreateProject() {
		NewProjectPage::NewProjectWin* win = new NewProjectPage::NewProjectWin();
		win->setWindowModality(Qt::ApplicationModal);
		win->show();
		win->setAttribute(Qt::WA_DeleteOnClose);
		connect(win, &NewProjectPage::NewProjectWin::projectPrepared, this, &ProjectWin::onOpenProject);
	}
	void ProjectWin::loadProject() {
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			delete project;
			project = nullptr;
		}
		HistoryProjectList.clear();
		QStringList okProjects;
		for (QString key : Config->keys("Project")) {
			QString projectPath = Config->getString("Project." + key);
			YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
			bool ok = project->loadProject(projectPath);
			if (!ok) {
				delete project;
				continue;
			}
			else {
				okProjects.append(projectPath);
				HistoryProjectList.append(project);
			}
		}
		Config->setArray("Project", okProjects);
		std::sort(HistoryProjectList.begin(), HistoryProjectList.end(), [](YSSCore::General::YSSProject* a, YSSCore::General::YSSProject* b) {
			return a->getProjectLastModifyTime() > b->getProjectLastModifyTime();
			});
		for (Visindigo::Widgets::MultiButton* label : HistoryProjectLabelList) {
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
			Visindigo::Widgets::MultiButton* label = new Visindigo::Widgets::MultiButton(HistoryProjectWidget);
			label->setObjectName("HistoryProject");
			label->setNormalStyleSheet(YSSTMSS("YSS::ProjectWin.HistoryProject.Normal"));
			label->setHoverStyleSheet(YSSTMSS("YSS::ProjectWin.HistoryProject.Hover"));
			label->setPressedStyleSheet(YSSTMSS("YSS::ProjectWin.HistoryProject.Pressed"));
			label->setTitle(project->getProjectName());
			QString lastModifyTime = project->getProjectLastModifyTime().toString("yyyy-MM-dd hh:mm:ss");
			label->setDescription(lastModifyTime + "\t" + project->getProjectFolder());
			HistoryProjectLabelList.append(label);
			HistoryProjectLayout->addWidget(label);
			label->setSpacing(5);
			label->setContentsMargins(10, 10, 10, 10);
			label->setToolTip(YSSTR("YSS::tooltips.projectWin.project"));
			label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			//label->setFixedHeight(50);
			label->show();
			HistoryProjectMap[label] = project;
			connect(label, &Visindigo::Widgets::MultiButton::clicked, this, &ProjectWin::onProjectSelected);
			connect(label, &Visindigo::Widgets::MultiButton::doubleClicked, this, &ProjectWin::onProjectDoubleClicked);
		}
		HistoryProjectWidget->setFixedHeight(HistoryProjectLabelList.length() * 100);
	}
}