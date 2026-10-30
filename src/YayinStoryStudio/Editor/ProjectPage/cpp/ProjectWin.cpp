#include <algorithm>
#include <QtCore/qelapsedtimer.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qtextbrowser.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/Log.h>
#include <General/Plugin.h>
#include <General/TranslationHost.h>
#include <General/Version.h>
#include <General/VIApplication.h>
#include <General/YSSLogger.h>
#include <General/YSSProject.h>
#include <General/YSSUniqueFileSystem.h>
#include <Utility/FileOperation.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include <Widgets/MultiButton.h>
#include <Widgets/MultiButtonGroup.h>
#include <Widgets/ThemeManager.h>
#include "Editor/MainEditor/MainWin.h"
#include "Editor/NewProjectPage/NewProjectWin.h"
#include "Editor/ProjectPage/ProjectInfoWidget.h"
#include "Editor/ProjectPage/ProjectWin.h"
#include "Editor/ProjectPage/OpenRemoteProjectWin.h"
#include "Editor/SSHConnection/SSHConnectionWidget.h"
#include "General/YSSURI.h"
#include "YayinStoryStudio.h"

VI_DECLARE_LOGGER(StartupProbe, YSSStartupProbe, YSS);

namespace YSS::ProjectPage {
	namespace {
		constexpr int OptionIconSize = 40;

		// 用于界面展示的项目位置文本：本地路径用原生分隔符，远端路径保留 ssh:// 形式。
		QString displayablePath(const YSSCore::General::YSSURI& uri) {
			const QString localPath = uri.getLocalPath();
			return localPath.isEmpty() ? uri.toString() : localPath;
		}
		QString recentProjectsKey() { return QStringLiteral("RecentProjects"); }
		QString remoteProjectInfoKey() { return QStringLiteral("RecentRemoteProjectInfo"); }
		QString remoteUriField() { return QStringLiteral("uri"); }
		QString remoteNameField() { return QStringLiteral("name"); }
		QString remoteCreateTimeField() { return QStringLiteral("createTime"); }
		QString remoteLastModifiedField() { return QStringLiteral("lastModified"); }
		// 远端项目没有缓存信息时统一使用的占位文本。
		QString unavailableText() { return VITRL("YSS::project.remoteInfoUnavailable"); }
		// RecentProjects 里的 URI，按存储顺序去重后返回。
		QStringList recentProjectUris() {
			Visindigo::Utility::JsonConfig* config = VIApp->getMainPlugin()->getPluginConfig();
			QStringList uris;
			for (const QString& stored : config->getStringList(recentProjectsKey())) {
				const QString trimmed = stored.trimmed();
				if (trimmed.isEmpty() || uris.contains(trimmed)) {
					continue;
				}
				uris.append(trimmed);
			}
			return uris;
		}
		// 从 RecentRemoteProjectInfo 里取出一条记录，找不到返回 false。
		bool findRemoteProjectInfo(const QString& uriText, QString* name,
			QDateTime* createTime, QDateTime* lastModified) {
			Visindigo::Utility::JsonConfig* config = VIApp->getMainPlugin()->getPluginConfig();
			const QList<Visindigo::Utility::JsonConfig> entries =
				config->getArray(remoteProjectInfoKey());
			for (const Visindigo::Utility::JsonConfig& entry : entries) {
				if (entry.getString(remoteUriField()) != uriText) {
					continue;
				}
				if (name != nullptr) {
					*name = entry.getString(remoteNameField());
				}
				if (createTime != nullptr) {
					*createTime = QDateTime::fromString(
						entry.getString(remoteCreateTimeField()), Qt::ISODate);
				}
				if (lastModified != nullptr) {
					*lastModified = QDateTime::fromString(
						entry.getString(remoteLastModifiedField()), Qt::ISODate);
				}
				return true;
			}
			return false;
		}
	}
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
				Visindigo::Utility::FileOperation::Errorable<QString> cacheResult = Visindigo::Utility::FileOperation::readAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_cache"));
				if (not cacheResult) {
					// 缓存不存在时视为与远端不一致，走下面的更新流程
					if (cacheResult.error() != Visindigo::Utility::FileOperation::FileNotFound) {
						yErrorF << "Failed to read news meta cache, error: " << Visindigo::Utility::FileOperation::errorCodeName(cacheResult.error());
					}
				}
				else if (jsonStr == cacheResult.value()) {
					Visindigo::Utility::FileOperation::Errorable<QString> contentResult = Visindigo::Utility::FileOperation::readAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_content"));
					if (not contentResult) {
						yErrorF << "Failed to read news content cache, error: " << Visindigo::Utility::FileOperation::errorCodeName(contentResult.error());
					}
					else if (not contentResult.value().isEmpty()) {
						NewsWidget->setMarkdown(contentResult.value());
					}
					bool needUpdate = VIApp->getMainPlugin()->getPluginConfig()->getBool("Update.Needed");
					QString latestVersion = VIApp->getMainPlugin()->getPluginConfig()->getString("Update.LatestVersion");
					if (needUpdate) {
						QString currentVersion = VIApp->getMainPlugin()->getPluginVersion().toString();
						QMessageBox::information(this, VITRL("YSS::update.newVersionAvailable"), 
							VITRL("YSS::update.newVersionAvailableDesc").arg(latestVersion).arg(currentVersion));
					}
					reply->deleteLater();
					return;
				}
				Visindigo::Utility::FileOperation::ErrorCode cacheSaveResult = Visindigo::Utility::FileOperation::saveAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_cache"), jsonStr);
				if (cacheSaveResult != Visindigo::Utility::FileOperation::Success) {
					yErrorF << "Failed to save news meta cache, error: " << Visindigo::Utility::FileOperation::errorCodeName(cacheSaveResult);
				}
				Visindigo::Utility::JsonConfig jsonConfig;
				if (jsonConfig.parse(jsonStr).error == QJsonParseError::NoError) {
					onNewsMetaGot(jsonConfig);
				}
			}
			else {
				NewsWidget->setMarkdown(VITRL("YSS::update.newsLoadFailed"));
				NewsWidget->setAlignment(Qt::AlignCenter);
			}
			reply->deleteLater();
			});

		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon.png"));
		this->setMinimumSize(1024, 576);
		this->setWindowTitle(VITRL("YSS::project.projectManager"));
		//this->setWindowFlags(Qt::ExpandedClientAreaHint | Qt::NoTitleBarBackgroundHint);
		TitleLabel = new Visindigo::Widgets::BorderLabel(this);
		TitleLabel->setContentsMargins(10, 0, 10, 0);
		TitleLabel->setText("Yayin Story Studio " + Visindigo::General::Version::getAPIVersion().toString());
		TitleLabel->setObjectName("ProgramTitleLabel");
		TitleLabel->setFixedHeight(80);
		auto font = TitleLabel->font();
		font.setPointSizeF(font.pointSizeF() * 2);
		TitleLabel->setFont(font);

		HistoryProjectArea = new QScrollArea(this);
		HistoryProjectArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
		HistoryProjectWidget = new QWidget(HistoryProjectArea);
		HistoryProjectWidget->setObjectName("HistoryProjectWidget");
		HistoryProjectWidget->setStyleSheet("QWidget#HistoryProjectWidget { background: transparent; }");
		HistoryProjectLayout = new QVBoxLayout(HistoryProjectWidget);
		HistoryProjectLayout->setContentsMargins(0, 0, 0, 0);
		HistoryProjectLayout->setSpacing(0);
		HistoryProjectWidget->setLayout(HistoryProjectLayout);
		HistoryProjectArea->setWidget(HistoryProjectWidget);
		HistoryProjectArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		HistoryProjectArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

		NewsWidget = new QTextBrowser(this);
		// 不设最大宽度：右侧栏由 QGridLayout 的列拉伸控制，这里再限宽会让宽窗口下留出空白。

		OptionWidget = new Visindigo::Widgets::BorderFrame(this);
		CreateProjectButton = new QToolButton(OptionWidget);
		CreateProjectButton->setText(VITRL("YSS::project.createNewProject"));
		CreateProjectButton->setToolTip(VITRL("YSS::tooltips.projectWin.createNewProject"));
		OpenFolderButton = new QToolButton(OptionWidget);
		OpenFolderButton->setText(VITRL("YSS::project.openFolder"));
		OpenFolderButton->setToolTip(VITRL("YSS::tooltips.projectWin.openFolder"));
		OpenRemoteButton = new QToolButton(OptionWidget);
		OpenRemoteButton->setText(VITRL("YSS::project.openRemoteProject"));
		OpenRemoteButton->setToolTip(VITRL("YSS::tooltips.projectWin.openRemoteProject"));
		CloneGitButton = new QPushButton(OptionWidget);
		CloneGitButton->setText(VITRL("YSS::project.cloneFromGit"));
		CloneGitButton->setToolTip(VITRL("YSS::tooltips.projectWin.cloneFromGit"));
		CloneGitButton->hide();
		SshConnectionButton = new QToolButton(OptionWidget);
		SshConnectionButton->setText(VITRL("YSS::sshConnection.title"));
		SshConnectionButton->setToolTip(VITRL("YSS::sshConnection.subtitle"));
		for (QToolButton* button : { CreateProjectButton, OpenFolderButton,
			OpenRemoteButton, SshConnectionButton }) {
			button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
			button->setProperty("projectAction", true);
			button->setIconSize(QSize(OptionIconSize, OptionIconSize));
			button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			button->setMinimumHeight(80);
			button->setCursor(Qt::PointingHandCursor);
		}
		ButtonLayout = new QGridLayout(OptionWidget);
		ButtonLayout->setContentsMargins(8, 8, 8, 8);
		ButtonLayout->setSpacing(6);
		ButtonLayout->addWidget(CreateProjectButton, 0, 0);
		ButtonLayout->addWidget(OpenFolderButton, 0, 1);
		ButtonLayout->addWidget(OpenRemoteButton, 1, 0);
		ButtonLayout->addWidget(SshConnectionButton, 1, 1);
		ButtonLayout->addWidget(CloneGitButton, 2, 0, 1, 2);
		ButtonLayout->setColumnStretch(0, 1);
		ButtonLayout->setColumnStretch(1, 1);
		OptionWidget->setLayout(ButtonLayout);
		refreshOptionIcons();
		InfoWidget = new ProjectInfoWidget(this);
		InfoWidget->setMinimumHeight(160);
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
		loadProject();

		int width = VIApp->getMainPlugin()->getPluginConfig()->getInt("Window.Project.Width");
		int height = VIApp->getMainPlugin()->getPluginConfig()->getInt("Window.Project.Height");

		this->resize(width, height);
		if (VIApp->getMainPlugin()->getPluginConfig()->getBool("Window.Project.Maximized")) {
			this->showMaximized();
		}
		connect(CreateProjectButton, &QToolButton::clicked, this, &ProjectWin::onCreateProject);
		connect(InfoWidget, &ProjectInfoWidget::removeConfirmed, this, &ProjectWin::onProjectRemoved);
		connect(OpenFolderButton, &QToolButton::clicked, this, &ProjectWin::onOpenProjectClicked);
		connect(OpenRemoteButton, &QToolButton::clicked, this, &ProjectWin::onOpenRemoteProjectClicked);
		connect(SshConnectionButton, &QToolButton::clicked, this, [this]() {
			YSS::Editor::SSHConnectionWidget* page = YSS::Editor::SSHConnectionWidget::getInstance();
			page->refreshConnectionList();
			page->show();
			page->raise();
			page->activateWindow();
			});
		setColorfulEnable(true);
		onThemeChanged();
		if (firstOpen) {
			firstOpen = false;
			QStringList launchArgs = qApp->arguments();
			if (launchArgs.size() > 1) {
				// 启动参数既可以是本地路径，也可以是 ssh:// 形式的远端项目，交给 YSSURI 统一解析。
				const YSSCore::General::YSSURI uri = YSSCore::General::YSSURI::fromString(launchArgs[1]);
				const QString localPath = uri.getLocalPath();
				const QString uriText = localPath.isEmpty() ? uri.toString() : localPath;
				if (uri.isValid()
					&& (uriText.endsWith(".yssp") || uriText.endsWith("yssproj.json"))) {
					onOpenProject(uri);
				}
				else {
					yMessageF << "Invalid project file path:" << launchArgs[1];
				}
			}
		}
	}

	void ProjectWin::closeEvent(QCloseEvent* event) {
		Visindigo::Utility::JsonConfig* config = VIApp->getMainPlugin()->getPluginConfig();
		config->setInt("Window.Project.Width", this->normalGeometry().width());
		config->setInt("Window.Project.Height", this->normalGeometry().height());
		config->setBool("Window.Project.Maximized", this->isMaximized());
		VIApp->getMainPlugin()->savePluginConfig();
		// 还在加载中的批次作废：它们的回调虽然会继续执行，但不会再碰本窗口的列表。
		++HistoryLoadGeneration;
		HistoryLoadResults.clear();
		for (const HistoryEntry& entry : HistoryProjectList) {
			delete entry.Project;
		}
		// 项目对象已经删掉了，容器不能再留着悬空指针，免得后续任何一次访问踩到已释放的对象。
		HistoryProjectList.clear();
		HistoryProjectMap.clear();
		HistoryProjectLabelList.clear();
		InfoWidget->showProject(nullptr);
		if (YSSCore::General::YSSProject::getCurrentProject()) {
			QElapsedTimer startupProbeTimer;
			startupProbeTimer.start();
			YSSStartupProbe->debug() << "ProjectWin -> MainWin transition started";
			if (YSS::Editor::MainWin::getInstance() == nullptr) {
				new YSS::Editor::MainWin();
			}
			YSS::Editor::MainWin::getInstance()->show();
			YSSStartupProbe->debug() << "ProjectWin -> MainWin transition finished, elapsed(ms):"
				<< startupProbeTimer.elapsed();
		}
		this->deleteLater();
	}

	void ProjectWin::resizeEvent(QResizeEvent* event) {
		HistoryProjectWidget->setFixedWidth(HistoryProjectArea->width() - 12);
	}

	void ProjectWin::onProjectRemoved(const YSSCore::General::YSSURI& uri) {
		const int index = indexOfHistory(uri.toString());
		if (index < 0) {
			// 面板持有的项目已经不在历史列表里（列表已重建或已被移除），不能拿-1去索引标签列表。
			InfoWidget->showProject(nullptr);
			return;
		}
		const QString uriText = HistoryProjectList[index].Uri.toString();
		QWidget* widget = HistoryProjectLabelList.value(index);
		if (widget != nullptr) {
			widget->hide();
			HistoryProjectLabelList.removeAt(index);
			HistoryProjectMap.remove(static_cast<Visindigo::Widgets::MultiButton*>(widget));
			HistoryProjectLayout->removeWidget(widget);
			widget->deleteLater();
		}
		YSSCore::General::YSSProject* project = HistoryProjectList[index].Project;
		HistoryProjectList.removeAt(index);
		QStringList remain;
		for (const QString& recentUri : recentProjectUris()) {
			if (recentUri != uriText) {
				remain.append(recentUri);
			}
		}
		Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
		Config->setArray(recentProjectsKey(), remain);
		this->pruneRemoteProjectInfo(remain);
		VIApp->getMainPlugin()->savePluginConfig();
		InfoWidget->showProject(nullptr);
		if (HistoryProjectLabelList.isEmpty()) {
			HistoryProjectWidget->setFixedHeight(0);
		}else{
			HistoryProjectWidget->setFixedHeight(HistoryProjectLabelList.length() * (HistoryProjectLabelList.first()->height() + HistoryProjectLayout->spacing()) + HistoryProjectLayout->contentsMargins().top() + HistoryProjectLayout->contentsMargins().bottom());
		}
		// 远端历史项目可能根本没加载过，这里拿到的是空指针。
		delete project;
		project = nullptr;
	}

	void ProjectWin::onProjectSelected() {
		Visindigo::Widgets::MultiButton* label = qobject_cast<Visindigo::Widgets::MultiButton*>(sender());
		if (label) {
			const int index = indexOfHistory(HistoryProjectMap.value(label));
			if (index < 0) {
				return;
			}
			const HistoryEntry& entry = HistoryProjectList[index];
			if (entry.Project != nullptr) {
				InfoWidget->showProject(entry.Project);
			}
			else {
				// 远端项目还没加载，按缓存信息展示；没有缓存时面板自己会写占位文本。
				InfoWidget->showUri(entry.Uri, entry.Name, entry.CreateTime, entry.LastModified);
			}
		}
	}

	void ProjectWin::onProjectDoubleClicked() {
		Visindigo::Widgets::MultiButton* label = qobject_cast<Visindigo::Widgets::MultiButton*>(sender());
		const int index = indexOfHistory(HistoryProjectMap.value(label));
		if (index < 0) {
			return;
		}
		YSSCore::General::YSSProject* project = HistoryProjectList[index].Project;
		if (project == nullptr) {
			// 远端历史项目还没有加载过：现在才需要连上去真正读一次。
			onOpenProject(HistoryProjectList[index].Uri);
			return;
		}
		if (not preCheckProject(project)) {
			return;
		}
		// 所有权移交给“当前项目”，必须先摘出历史列表，否则closeEvent会把它一起删掉。
		HistoryProjectList.removeAt(index);
		HistoryProjectMap.remove(label);
		HistoryProjectLabelList.removeAll(label);
		HistoryProjectLayout->removeWidget(label);
		label->deleteLater();
		YSSCore::General::YSSProject::setCurrentProject(project);
		this->close();
	}

	void ProjectWin::onOpenProjectClicked() {
		const QString filePath = QFileDialog::getOpenFileName(this,
			VITRL("YSS::project.openYSSProject"), "./user_data/repos",
			"YSS Project (*.yssp);;YSS Project (yssproj.json)");
		if (filePath.isEmpty()) {
			return;
		}
		onOpenProject(YSSCore::General::YSSURI::fromLocalPath(filePath));
	}

	void ProjectWin::onOpenRemoteProjectClicked() {
		OpenRemoteProjectWin* win = new OpenRemoteProjectWin(this);
		win->setAttribute(Qt::WA_DeleteOnClose);
		win->setWindowModality(Qt::ApplicationModal);
		connect(win, &OpenRemoteProjectWin::projectChosen, this,
			[this](const YSSCore::General::YSSURI& uri) {
				onOpenProject(uri);
			});
		win->show();
	}

	void ProjectWin::onOpenProject(const YSSCore::General::YSSURI& uri) {
		if (not uri.isValid()) {
			return;
		}
		if (not ensureRemoteConnected(uri)) {
			return;
		}
		YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
		// 加载是异步的，结论只能从信号里拿；这里只连接一次，之后本窗口不再关心这个对象。
		connect(project, &YSSCore::General::YSSProject::projectLoaded, this, [this, project]() {
			Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
			const QString projectPath = project->getProjectPath().toString();
			QStringList recent = recentProjectUris();
			if (not recent.contains(projectPath)) {
				yMessageF << "Add" << projectPath << "to project list.";
				recent.append(projectPath);
				Config->setArray(recentProjectsKey(), recent);
			}
			// 远端项目要留一份显示用的信息，下次未连接时列表才不会空着。
			this->rememberRemoteProjectInfo(project);
			if (not preCheckProject(project)) {
				project->deleteLater();
				return;
			}
			YSSCore::General::YSSProject::setCurrentProject(project);
			this->close();
			});
		connect(project, &YSSCore::General::YSSProject::projectLoadFailed, this,
			[this, project](YSSCore::General::YSSProject::LoadError error) {
				QMessageBox msgBox;
				msgBox.setIcon(QMessageBox::Critical);
				msgBox.setText(VITRL("YSS::project.openProjectFailed.message"));
				switch (error) {
				case YSSCore::General::YSSProject::LoadError::NotFound:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.NotFound"));
					break;
				case YSSCore::General::YSSProject::LoadError::ReadFailed:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.ReadFailed"));
					break;
				case YSSCore::General::YSSProject::LoadError::ParseError:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.ParseError"));
					break;
				case YSSCore::General::YSSProject::LoadError::InvalidConfig:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.InvalidConfig"));
					break;
				default:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.UnknownError"));
					break;
				}
				msgBox.setStandardButtons(QMessageBox::Ok);
				msgBox.exec();
				project->deleteLater();
			});
		project->loadProject(uri);
	}

	/*
		检查 \a project 是否满足打开条件：所需插件是否齐全、项目版本是否兼容。

		本函数只检查和询问，不接管 \a project 的所有权。
		return 用户选择继续打开返回true，选择取消返回false。
	*/
	bool ProjectWin::preCheckProject(YSSCore::General::YSSProject* project) {
		auto requiredPlugins = project->getRequiredPlugins();
		bool allPluginsAvailable = true;
		QStringList missingPlugins;
		for (auto& pluginID : requiredPlugins) {
			if (not VIPLM->isPluginEnable(pluginID)) {
				allPluginsAvailable = false;
				missingPlugins << pluginID;
			}
		}
		if (not allPluginsAvailable) {
			QMessageBox msgBox;
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.setWindowTitle(VITRL("YSS::project.requiredPluginsNotAvailable.title"));
			msgBox.setText(VITRL("YSS::project.requiredPluginsNotAvailable.message").arg(project->getProjectName()) +
				"\n" + missingPlugins.join("\n"));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			auto rtn = msgBox.exec();
			if (rtn == QMessageBox::No) {
				return false;
			}
		}
		auto projectVersion = project->getProjectVersion();
		yDebug << "Project version:" << projectVersion.toString() << ", Current version:" << YSSCore::General::YSSProject::getCurrentProjectVersion().toString();
		if (projectVersion != YSSCore::General::YSSProject::getCurrentProjectVersion()) {
			if (projectVersion < YSSCore::General::YSSProject::getCurrentProjectVersion()) {
				QMessageBox msgBox;
				msgBox.setIcon(QMessageBox::Warning);
				msgBox.setWindowTitle(VITRL("YSS::project.projectVersionTooOld.title"));
				auto versionRange = YSSCore::General::YSSProject::getProjectVersionRange();
				auto leftVersion = std::get<0>(versionRange.value(projectVersion));
				auto rightVersion = std::get<1>(versionRange.value(projectVersion));
				msgBox.setText(VITRL("YSS::project.projectVersionTooOld.message").arg(project->getProjectName()).arg(projectVersion.toString()).arg(leftVersion.toString()).arg(rightVersion.toString()));
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msgBox.setDefaultButton(QMessageBox::No);
				auto rtn = msgBox.exec();
				if (rtn == QMessageBox::No) {
					return false;
				}
				project->updateProject();
			}
			else {
				QMessageBox msgBox;
				msgBox.setIcon(QMessageBox::Warning);
				msgBox.setWindowTitle(VITRL("YSS::project.projectVersionTooNew.title"));
				msgBox.setText(VITRL("YSS::project.projectVersionTooNew.message").arg(project->getProjectName()));
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				auto rtn = msgBox.exec();
				if (rtn == QMessageBox::No) {
					return false;
				}
			}
		}
		return true;
	}
	void ProjectWin::onCreateProject() {
		NewProjectPage::NewProjectWin* win = new NewProjectPage::NewProjectWin();
		win->setAttribute(Qt::WA_DeleteOnClose);
		win->setWindowModality(Qt::ApplicationModal);
		win->setWindowFlags(win->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
		win->show();
		connect(win, &NewProjectPage::NewProjectWin::projectPrepared, this,
			[this](const YSSCore::General::YSSURI& projectUri) {
				this->onOpenProject(projectUri);
			});
	}

	void ProjectWin::loadProject() {
		for (const HistoryEntry& entry : HistoryProjectList) {
			delete entry.Project;
		}
		HistoryProjectList.clear();
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

		HistoryLoadOrder = recentProjectUris();
		// 加载是异步的，一批加载全部结束之后再重建列表。用代数丢弃过期批次的结果，
		// 避免两次 loadProject 的完成顺序交错时把旧结果写进新列表。
		++HistoryLoadGeneration;
		const int generation = HistoryLoadGeneration;
		HistoryLoadResults.clear();
		HistoryLoadPending = 0;
		for (const QString& uriText : HistoryLoadOrder) {
			const YSSCore::General::YSSURI uri = YSSCore::General::YSSURI::fromString(uriText);
			if (not uri.isValid()) {
				continue;
			}
			// 远端项目在没建立连接时加载必然失败，而失败会被当成“项目已不存在”从列表里剔掉，
			// 所以远端条目启动时只按缓存展示，等用户真正打开时再加载。
			if (uri.isRemote()) {
				continue;
			}
			++HistoryLoadPending;
			YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
			connect(project, &YSSCore::General::YSSProject::projectLoaded, this, [this, project, generation]() {
				if (generation != HistoryLoadGeneration) {
					project->deleteLater();
					return;
				}
				HistoryLoadResults.append(project);
				if (--HistoryLoadPending == 0) {
					finishHistoryLoad(generation);
				}
				});
			connect(project, &YSSCore::General::YSSProject::projectLoadFailed, this, [this, project, generation]() {
				project->deleteLater();
				if (generation != HistoryLoadGeneration) {
					return;
				}
				if (--HistoryLoadPending == 0) {
					finishHistoryLoad(generation);
				}
				});
			project->loadProject(uri);
		}
		if (HistoryLoadPending == 0) {
			finishHistoryLoad(generation);
		}
	}

	/*
		一批历史项目加载完成后重建列表。

		本地项目只有加载成功才会保留（原行为）；远端项目即使没有加载也会保留，
		名称与时间优先用 RecentRemoteProjectInfo 里的缓存，没有缓存就写占位文本。
	*/
	void ProjectWin::finishHistoryLoad(int generation) {
		if (generation != HistoryLoadGeneration) {
			return;
		}
		QMap<QString, YSSCore::General::YSSProject*> loadedProjects;
		for (YSSCore::General::YSSProject* project : HistoryLoadResults) {
			loadedProjects.insert(project->getProjectPath().toString(), project);
		}
		HistoryLoadResults.clear();

		QList<HistoryEntry> entries;
		QStringList keepUris;
		for (const QString& uriText : HistoryLoadOrder) {
			HistoryEntry entry;
			entry.Uri = YSSCore::General::YSSURI::fromString(uriText);
			if (not entry.Uri.isValid()) {
				continue;
			}
			YSSCore::General::YSSProject* project = loadedProjects.take(uriText);
			if (project != nullptr) {
				entry.Project = project;
				entry.Name = project->getProjectName();
				entry.CreateTime = project->getProjectCreateTime();
				entry.LastModified = project->getProjectLastModifyTime();
			}
			else if (entry.Uri.isRemote()) {
				QString name;
				QDateTime createTime;
				QDateTime lastModified;
				if (findRemoteProjectInfo(uriText, &name, &createTime, &lastModified)) {
					entry.Name = name;
					entry.CreateTime = createTime;
					entry.LastModified = lastModified;
				}
				else {
					entry.Name = unavailableText();
				}
			}
			else {
				// 本地项目加载失败：保持原行为，不再留在列表里。
				continue;
			}
			entries.append(entry);
			keepUris.append(uriText);
		}
		// 配置里已不再引用的项目对象直接释放。
		for (YSSCore::General::YSSProject* leftover : loadedProjects) {
			delete leftover;
		}

		// 时间未知的条目排在最后，其余按最后修改时间倒序。
		std::sort(entries.begin(), entries.end(), [](const HistoryEntry& a, const HistoryEntry& b) {
			if (a.LastModified.isValid() != b.LastModified.isValid()) {
				return a.LastModified.isValid();
			}
			return a.LastModified > b.LastModified;
			});
		HistoryProjectList = entries;

		Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
		Config->setArray(recentProjectsKey(), keepUris);
		this->pruneRemoteProjectInfo(keepUris);
		VIApp->getMainPlugin()->savePluginConfig();

		const QString unavailable = unavailableText();
		for (const HistoryEntry& entry : HistoryProjectList) {
			Visindigo::Widgets::MultiButton* label = new Visindigo::Widgets::MultiButton(HistoryProjectWidget);
			label->setObjectName("HistoryProject");
			label->setTitle(entry.Name);
			const QString lastModifyTime = entry.LastModified.isValid()
				? entry.LastModified.toString("yyyy-MM-dd hh:mm:ss")
				: unavailable;
			const QString location = entry.Project != nullptr
				? displayablePath(entry.Project->getProjectFolder())
				: displayablePath(entry.Uri.parent());
			label->setDescription(lastModifyTime + "\n" + location);
			HistoryProjectLabelList.append(label);
			HistoryProjectLayout->addWidget(label);
			label->setToolTip(VITRL("YSS::tooltips.projectWin.project"));
			label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			label->show();
			HistoryProjectMap[label] = entry.Uri.toString();
			connect(label, &Visindigo::Widgets::MultiButton::clicked, this, &ProjectWin::onProjectSelected);
			connect(label, &Visindigo::Widgets::MultiButton::doubleClicked, this, &ProjectWin::onProjectDoubleClicked);
		}
		HistoryProjectWidget->setFixedHeight(HistoryProjectLabelList.length() * 90);
	}

	int ProjectWin::indexOfHistory(const QString& uriText) const {
		if (uriText.isEmpty()) {
			return -1;
		}
		for (int i = 0; i < HistoryProjectList.size(); i++) {
			if (HistoryProjectList[i].Uri.toString() == uriText) {
				return i;
			}
		}
		return -1;
	}

	bool ProjectWin::ensureRemoteConnected(const YSSCore::General::YSSURI& uri) {
		if (not uri.isRemote()) {
			return true;
		}
		if (YSSCore::General::YSSUniqueFileSystem::getInstance()->isReady(uri)) {
			return true;
		}
		YSS::Editor::SSHConnectionWidget::requestVerification(uri);
		return false;
	}

	void ProjectWin::rememberRemoteProjectInfo(YSSCore::General::YSSProject* project) {
		const YSSCore::General::YSSURI uri = project->getProjectPath();
		if (not uri.isRemote()) {
			return;
		}
		const QString uriText = uri.toString();
		Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
		QList<Visindigo::Utility::JsonConfig> entries = Config->getArray(remoteProjectInfoKey());
		bool replaced = false;
		for (Visindigo::Utility::JsonConfig& entry : entries) {
			if (entry.getString(remoteUriField()) != uriText) {
				continue;
			}
			entry.setString(remoteNameField(), project->getProjectName());
			entry.setString(remoteCreateTimeField(),
				project->getProjectCreateTime().toString(Qt::ISODate));
			entry.setString(remoteLastModifiedField(),
				project->getProjectLastModifyTime().toString(Qt::ISODate));
			replaced = true;
			break;
		}
		if (not replaced) {
			Visindigo::Utility::JsonConfig entry;
			entry.setString(remoteUriField(), uriText);
			entry.setString(remoteNameField(), project->getProjectName());
			entry.setString(remoteCreateTimeField(),
				project->getProjectCreateTime().toString(Qt::ISODate));
			entry.setString(remoteLastModifiedField(),
				project->getProjectLastModifyTime().toString(Qt::ISODate));
			entries.append(entry);
		}
		Config->setArray(remoteProjectInfoKey(), entries);
	}

	void ProjectWin::pruneRemoteProjectInfo(const QStringList& keepUris) {
		Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
		const QList<Visindigo::Utility::JsonConfig> entries =
			Config->getArray(remoteProjectInfoKey());
		QList<Visindigo::Utility::JsonConfig> kept;
		for (const Visindigo::Utility::JsonConfig& entry : entries) {
			if (keepUris.contains(entry.getString(remoteUriField()))) {
				kept.append(entry);
			}
		}
		if (kept.size() == entries.size()) {
			return;
		}
		Config->setArray(remoteProjectInfoKey(), kept);
	}

	void ProjectWin::onThemeChanged() {
		if (OptionWidget != nullptr) {
			OptionWidget->setStyleSheet(VISTMGT("YSS::ProjectPage.ProjectOption", OptionWidget));
			refreshOptionIcons();
		}
		for (Visindigo::Widgets::MultiButton* label : HistoryProjectLabelList) {

		}
	}

	void ProjectWin::refreshOptionIcons() {
		const QColor iconColor = this->palette().color(QPalette::WindowText);
		CreateProjectButton->setIcon(VIApp->getNamedFontIcon("Add", OptionIconSize, { iconColor }));
		OpenFolderButton->setIcon(VIApp->getNamedFontIcon("OpenFile", OptionIconSize, { iconColor }));
		OpenRemoteButton->setIcon(VIApp->getNamedFontIcon("Cloud", OptionIconSize, { iconColor }));
		SshConnectionButton->setIcon(VIApp->getNamedFontIcon("Connect", OptionIconSize, { iconColor }));
	}

	void ProjectWin::onNewsMetaGot(const Visindigo::Utility::JsonConfig& config) {
		QString versionKey = "version";
		QString updateNewsKey = "update_news";
		QString newsKey = "news";
		QString updateChannel = VIApp->getMainPlugin()->getPluginConfig()->getString("Settings.General.UpdateChannel");
		if (updateChannel == "beta") {
			versionKey = "version_beta";
			updateNewsKey = "update_news_beta";
			newsKey = "news_beta";
		}
		else if (updateChannel == "tp") {
			versionKey = "version_tp";
			updateNewsKey = "update_news_tp";
			newsKey = "news_tp";
		}
		else {
			VIApp->getMainPlugin()->getPluginConfig()->setString("Settings.General.UpdateChannel", "release");
		}
		if (config.contains(versionKey)) {
			QString version = config.getString(versionKey);
			Visindigo::General::Version metaVersion(version);
			auto req = QNetworkRequest();
			bool isUpdate = metaVersion > VIApp->getMainPlugin()->getPluginVersion();
			if (isUpdate) {
				req = QNetworkRequest(QUrl(config.getString(updateNewsKey)));
			}
			else {
				req = QNetworkRequest(QUrl(config.getString(newsKey)));
			}
			QNetworkReply* reply = NetworkManager->get(req);
			connect(reply, &QNetworkReply::finished, this, [this, reply, isUpdate, metaVersion]() {
				if (reply->error() == QNetworkReply::NoError) {
					QByteArray responseData = reply->readAll();
					QString newsStr = QString::fromUtf8(responseData);
					NewsWidget->setMarkdown(newsStr);
					Visindigo::Utility::FileOperation::ErrorCode contentSaveResult = Visindigo::Utility::FileOperation::saveAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_content"), newsStr);
					if (contentSaveResult != Visindigo::Utility::FileOperation::Success) {
						yErrorF << "Failed to save news content cache, error: " << Visindigo::Utility::FileOperation::errorCodeName(contentSaveResult);
					}
					VIApp->getMainPlugin()->getPluginConfig()->setBool("Update.Needed", isUpdate);
					VIApp->getMainPlugin()->getPluginConfig()->setString("Update.LatestVersion", metaVersion.toString());
					if (isUpdate) {
						auto version = YSS::Main::getInstance()->getPluginVersion().toString();
						QMessageBox::information(this, VITRL("YSS::update.newVersionAvailable"), 
							VITRL("YSS::update.newVersionAvailableDesc").arg(metaVersion.toString()).arg(version));
					}
				}
				reply->deleteLater();
				});
		}
	}
}