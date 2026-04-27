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
#include "Editor/MainEditor/MainWin.h"
#include <General/Log.h>
#include <QtWidgets/qfiledialog.h>
#include "Editor/NewProjectPage/NewProjectWin.h"
#include <General/YSSLogger.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qtextbrowser.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <General/Version.h>
#include <General/VIApplication.h>
#include <General/Plugin.h>
#include <Widgets/MultiButtonGroup.h>
namespace YSS::ProjectPage {
	bool ProjectWin::firstOpen = true;
	ProjectWin::ProjectWin() :QFrame() {
		NetworkManager = new QNetworkAccessManager(this);
		// access http://yxgeneral.cn/DirectFiles/YSS/meta.json to get news info
		auto request = QNetworkRequest(QUrl("http://yxgeneral.cn/DirectFiles/YSS/meta.json"));
		QNetworkReply* reply = NetworkManager->get(request);
		connect(reply, &QNetworkReply::finished, this, [this, reply]() {
			if (reply->error() == QNetworkReply::NoError) {
				QByteArray responseData = reply->readAll();
				QString jsonStr = QString::fromUtf8(responseData);
				jsonStr = jsonStr.replace("\r", "");
				QString cacheStr = Visindigo::Utility::FileUtility::readAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_cache"));
				if (jsonStr == cacheStr) {
					QString content = Visindigo::Utility::FileUtility::readAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_content"));
					if (!content.isEmpty()) {
						NewsWidget->setMarkdown(content);
					}
					reply->deleteLater();
					return;
				}
				Visindigo::Utility::FileUtility::saveAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_cache"), jsonStr);
				Visindigo::Utility::JsonConfig jsonConfig;
				if (jsonConfig.parse(jsonStr).error == QJsonParseError::NoError) {
					onNewsMetaGot(jsonConfig);
				}
			}
			else {
				NewsWidget->setMarkdown(VITR("YSS::project.newsLoadFailed"));
				NewsWidget->setAlignment(Qt::AlignCenter);
			}
			reply->deleteLater();
			});

		this->setAttribute(Qt::WA_TranslucentBackground);
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/yssicon.png"));
		this->setMinimumSize(1366, 768);
		this->setWindowTitle(VITR("YSS::project.projectManager"));
		//this->setWindowFlags(Qt::ExpandedClientAreaHint | Qt::NoTitleBarBackgroundHint);
		TitleLabel = new QLabel(this);
		TitleLabel->setText(" Yayin Story Studio " + Visindigo::General::Version::getAPIVersion().toString());
		TitleLabel->setObjectName("ProgramTitleLabel");
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

		NewsWidget = new QTextBrowser(this);
		NewsWidget->setMaximumWidth(300);

		OptionWidget = new Visindigo::Widgets::BorderFrame(this);
		CreateProjectButton = new QPushButton(OptionWidget);
		CreateProjectButton->setText(VITR("YSS::project.createNewProject"));
		CreateProjectButton->setToolTip(VITR("YSS::tooltips.projectWin.createNewProject"));
		OpenFolderButton = new QPushButton(OptionWidget);
		OpenFolderButton->setText(VITR("YSS::project.openFolder"));
		OpenFolderButton->setToolTip(VITR("YSS::tooltips.projectWin.openFolder"));
		CloneGitButton = new QPushButton(OptionWidget);
		CloneGitButton->setText(VITR("YSS::project.cloneFromGit"));
		CloneGitButton->setToolTip(VITR("YSS::tooltips.projectWin.cloneFromGit"));
		CloneGitButton->hide();
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
		//this->HistoryProjectArea->setStyleSheet(VISTMGT("YSS::NormalScrollBar", this));
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
		setColorfulEnable(true);
		onThemeChanged();
		if (firstOpen) {
			firstOpen = false;
			QStringList launchArgs = qApp->arguments();
			if (launchArgs.size() > 1) {
				QString ysspFilePath = launchArgs[1];
				if (ysspFilePath.endsWith(".yssp") || ysspFilePath.endsWith("yssproj.json")) {
					onOpenProject(ysspFilePath);
				}
				else {
					yMessageF << "Invalid project file path:" << ysspFilePath;
				}
			}
		}
	}

	void ProjectWin::closeEvent(QCloseEvent* event) {
		Visindigo::Utility::JsonConfig* config = GlobalValue::getConfig();
		config->setInt("Window.Project.Width", this->normalGeometry().width());
		config->setInt("Window.Project.Height", this->normalGeometry().height());
		config->setBool("Window.Project.Maximized", this->isMaximized());
		GlobalValue::saveConfig();
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			delete project;
			project = nullptr;
		}
		if (GlobalValue::getCurrentProject()) {
			if (YSS::Editor::MainWin::getInstance() == nullptr) {
				new YSS::Editor::MainWin();
			}
			YSS::Editor::MainWin::getInstance()->show();
		}
		this->deleteLater();
	}

	void ProjectWin::resizeEvent(QResizeEvent* event) {
		HistoryProjectWidget->setFixedWidth(HistoryProjectArea->width() - 12* this->devicePixelRatioF());
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
		Visindigo::Utility::JsonConfig* Config = GlobalValue::getConfig();
		QStringList keys = Config->keys("RecentProjects");
		for (int i = 0; i < keys.size(); i++) {
			if (Config->getString("RecentProjects." + keys[i]) == project->getProjectPath()) {
				yMessageF << i;
				Config->remove("RecentProjects." + keys[i]);
				break;
			}
		}
		GlobalValue::saveConfig();

		HistoryProjectWidget->setFixedHeight(HistoryProjectLabelList.length() * (HistoryProjectLabelList.first()->height() + HistoryProjectLayout->spacing()) + HistoryProjectLayout->contentsMargins().top() + HistoryProjectLayout->contentsMargins().bottom());
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
			filePath = QFileDialog::getOpenFileName(this, VITR("YSS::project.openYSSProject"), "./user_data/repos", "YSS Project (*.yssp);;YSS Project (yssproj.json)");
		}
		filePath = Visindigo::Utility::FileUtility::getRelativeIfStartWith(QDir::currentPath(), filePath);
		Visindigo::Utility::JsonConfig* Config = GlobalValue::getConfig();
		YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
		YSSCore::General::YSSProject::LoadProjectResult res = project->loadProject(filePath);
		if (res == YSSCore::General::YSSProject::Success) {
			bool inList = false;
			for (QString key : Config->keys("RecentProjects")) {
				QString projectPath = Config->getString("RecentProjects." + key);
				if (projectPath == filePath) { inList = true; break; }
			}
			if (!inList) {
				yMessageF << "Add" << filePath << "to project list.";
				Config->setString("RecentProjects." + QString::number(Config->keys("RecentProjects").size()), filePath);
			}
			GlobalValue::setCurrentProject(project);
			this->close();
		}
		else {
			QMessageBox msgBox;
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.setText(VITR("YSS::project.openProjectFailed.message"));
			switch (res) {
			case YSSCore::General::YSSProject::ParseError:
				msgBox.setInformativeText(VITR("YSS::project.openProjectFailed.ParseError"));
				break;
			case YSSCore::General::YSSProject::InvalidConfig:
				msgBox.setInformativeText(VITR("YSS::project.openProjectFailed.InvalidConfig"));
				break;
			default:
				msgBox.setInformativeText(VITR("YSS::project.openProjectFailed.UnknownError"));
				break;
			}
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.exec();
			delete project;
		}
	}

	void ProjectWin::onCreateProject() {
		NewProjectPage::NewProjectWin* win = new NewProjectPage::NewProjectWin();
		win->setAttribute(Qt::WA_DeleteOnClose);
		win->setWindowModality(Qt::ApplicationModal);
		win->setWindowFlags(win->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
		win->show();
		connect(win, &NewProjectPage::NewProjectWin::projectPrepared, this, &ProjectWin::onOpenProject);
	}

	void ProjectWin::loadProject() {
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			delete project;
			project = nullptr;
		}
		HistoryProjectList.clear();
		QStringList okProjects;
		Visindigo::Utility::JsonConfig* Config = GlobalValue::getConfig();
		for (QString key : Config->keys("RecentProjects")) {
			QString projectPath = Config->getString("RecentProjects." + key);
			YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
			YSSCore::General::YSSProject::LoadProjectResult res = project->loadProject(projectPath);
			if (res!=YSSCore::General::YSSProject::Success) {
				delete project;
				continue;
			}
			else {
				okProjects.append(projectPath);
				HistoryProjectList.append(project);
			}
		}
		Config->setArray("RecentProjects", okProjects);
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
			label->setTitle(project->getProjectName());
			QString lastModifyTime = project->getProjectLastModifyTime().toString("yyyy-MM-dd hh:mm:ss");
			label->setDescription(lastModifyTime + "\t" + project->getProjectFolder());
			HistoryProjectLabelList.append(label);
			HistoryProjectLayout->addWidget(label);
			//label->setSpacing(5);
			//label->setContentsMargins(10, 10, 10, 10);
			label->setToolTip(VITR("YSS::tooltips.projectWin.project")); 
			label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			//label->setFixedHeight(50);
			label->show();
			HistoryProjectMap[label] = project;
			connect(label, &Visindigo::Widgets::MultiButton::clicked, this, &ProjectWin::onProjectSelected);
			connect(label, &Visindigo::Widgets::MultiButton::doubleClicked, this, &ProjectWin::onProjectDoubleClicked);
		}
		HistoryProjectWidget->setFixedHeight(HistoryProjectLabelList.length() * 80);
	}

	void ProjectWin::onThemeChanged() {
		//yDebugF << VISTMGT("YSS::General.MultiButton.Normal",this);
		//this->applyVIStyleTemplate("YSS::ProjectWin");
		for (Visindigo::Widgets::MultiButton* label : HistoryProjectLabelList) {
			//label->setNormalStyleSheet(VISTMGT("YSS::General.MultiButton.Normal"));
			//label->setHoverStyleSheet(VISTMGT("YSS::General.MultiButton.Hover"));
			//label->setPressedStyleSheet(VISTMGT("YSS::General.MultiButton.Pressed"));
		}
	}

	void ProjectWin::onNewsMetaGot(const Visindigo::Utility::JsonConfig& config) {
		if (config.contains("version")) {
			QString version = config.getString("version");
			Visindigo::General::Version metaVersion(version);
			auto req = QNetworkRequest();
			if (metaVersion > VIApp->getMainPlugin()->getPluginVersion()) {
				req = QNetworkRequest(QUrl(config.getString("update_news")));
			}
			else {
				req = QNetworkRequest(QUrl(config.getString("news")));
			}
			QNetworkReply* reply = NetworkManager->get(req);
			connect(reply, &QNetworkReply::finished, this, [this, reply]() {
				if (reply->error() == QNetworkReply::NoError) {
					QByteArray responseData = reply->readAll();
					QString newsStr = QString::fromUtf8(responseData);
					NewsWidget->setMarkdown(newsStr);
					Visindigo::Utility::FileUtility::saveAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_content"), newsStr);
				}
				reply->deleteLater();
				});
		}
	}
}