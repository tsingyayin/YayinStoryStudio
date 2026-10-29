#include <algorithm>
#include <functional>
#include <QtCore/qlist.h>
#include <QtCore/qset.h>
#include <QtGui/qicon.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qscrollbar.h>
#include <Editor/ProjectTemplateManager.h>
#include <Editor/ProjectTemplateProvider.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/YSSLogger.h>
#include <General/YSSProject.h>
#include <General/YSSUniqueFileSystem.h>
#include <Utility/JsonConfig.h>
#include <Widgets/BorderLabel.h>
#include <Widgets/MultiButton.h>
#include <Widgets/ThemeManager.h>
#include "Editor/NewProjectPage/NewProjectWin.h"
#include "Editor/NewProjectPage/ProjectLocateWin.h"

namespace YSS::NewProjectPage {
	namespace {
		// 按模板给出的内容异步创建项目：建目录、写文件、保存 project.yssp，全部走统一文件系统。
		class ProjectCreationJob :public QObject {
		public:
			ProjectCreationJob(const YSSCore::General::YSSURI& targetUri,
				const QMap<QString, QByteArray>& files,
				const Visindigo::Utility::JsonConfig& content,
				const QStringList& openFiles, QObject* parent)
				: QObject(parent), TargetUri(targetUri), Files(files), Content(content),
				OpenFiles(openFiles) {
			}
			std::function<void()> OnSucceeded;
			std::function<void(const QString&, bool)> OnFailed;
			YSSCore::General::YSSURI getProjectConfigUri() const { return TargetUri.resolve(QStringLiteral("project.yssp")); }
			void start() {
				checkTarget();
			}
		private:
			void checkTarget() {
				YSSCore::General::YSSFileSystemReply* reply =
					YSSCore::General::YSSUniqueFileSystem::getInstance()->list(TargetUri);
				if (reply == nullptr) {
					fail(TargetUri.toString(), false);
					return;
				}
				connect(reply, &YSSCore::General::YSSFileSystemReply::finished, this, [this, reply]() {
					const YSSCore::General::YSSFileError error = reply->getError();
					if (!error.isError() and !reply->getEntries().isEmpty()) {
						fail(TargetUri.toString(), true);
						return;
					}
					collectDirectories();
					nextMkdir();
					});
			}
			void collectDirectories() {
				QSet<QString> dirs;
				dirs.insert(QString());
				for (const QString& path : Files.keys()) {
					QStringList parts = path.split(QLatin1Char('/'));
					parts.removeLast();
					QString current;
					for (const QString& part : parts) {
						if (part.isEmpty()) {
							continue;
						}
						current = current.isEmpty() ? part : current + QLatin1Char('/') + part;
						dirs.insert(current);
					}
				}
				QStringList sorted = dirs.values();
				std::sort(sorted.begin(), sorted.end(), [](const QString& left, const QString& right) {
					const qsizetype leftDepth = left.isEmpty() ? 0 : left.count(QLatin1Char('/')) + 1;
					const qsizetype rightDepth = right.isEmpty() ? 0 : right.count(QLatin1Char('/')) + 1;
					if (leftDepth != rightDepth) {
						return leftDepth < rightDepth;
					}
					return left < right;
					});
				DirQueue = sorted;
			}
			void nextMkdir() {
				if (DirQueue.isEmpty()) {
					FileQueue = Files.keys();
					nextWrite();
					return;
				}
				const QString relative = DirQueue.takeFirst();
				const YSSCore::General::YSSURI uri = relative.isEmpty() ? TargetUri : TargetUri.resolve(relative);
				YSSCore::General::YSSFileSystemReply* reply =
					YSSCore::General::YSSUniqueFileSystem::getInstance()->mkdir(uri);
				if (reply == nullptr) {
					fail(uri.toString(), false);
					return;
				}
				connect(reply, &YSSCore::General::YSSFileSystemReply::finished, this, [this, reply, uri]() {
					const YSSCore::General::YSSFileError error = reply->getError();
					if (error.isError()
						and error.Type != YSSCore::General::YSSFileError::ErrorType::AlreadyExists) {
						fail(uri.toString(), false);
						return;
					}
					nextMkdir();
					});
			}
			void nextWrite() {
				if (FileQueue.isEmpty()) {
					saveProjectConfig();
					return;
				}
				const QString relative = FileQueue.takeFirst();
				const YSSCore::General::YSSURI uri = TargetUri.resolve(relative);
				YSSCore::General::YSSFileSystemReply* reply =
					YSSCore::General::YSSUniqueFileSystem::getInstance()->write(uri, Files.value(relative));
				if (reply == nullptr) {
					fail(uri.toString(), false);
					return;
				}
				connect(reply, &YSSCore::General::YSSFileSystemReply::finished, this, [this, reply, uri]() {
					const YSSCore::General::YSSFileError error = reply->getError();
					if (error.isError()) {
						fail(uri.toString(), false);
						return;
					}
					nextWrite();
					});
			}
			void saveProjectConfig() {
				YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
				project->setYSSProjectContent(Content);
				for (const QString& relative : OpenFiles) {
					project->addEditorOpenedFile(TargetUri.resolve(relative));
				}
				if (!OpenFiles.isEmpty()) {
					project->setFocusedFile(TargetUri.resolve(OpenFiles.first()));
				}
				connect(project, &YSSCore::General::YSSProject::projectSaved, this, [this, project]() {
					project->deleteLater();
					if (OnSucceeded) {
						OnSucceeded();
					}
					this->deleteLater();
					});
				connect(project, &YSSCore::General::YSSProject::projectSaveFailed, this, [this, project]() {
					project->deleteLater();
					fail(TargetUri.resolve(QStringLiteral("project.yssp")).toString(), false);
					});
				project->saveProject(TargetUri.resolve(QStringLiteral("project.yssp")));
			}
			void fail(const QString& target, bool alreadyExists) {
				if (OnFailed) {
					OnFailed(target, alreadyExists);
				}
				this->deleteLater();
			}
		private:
			YSSCore::General::YSSURI TargetUri;
			QMap<QString, QByteArray> Files;
			Visindigo::Utility::JsonConfig Content;
			QStringList OpenFiles;
			QStringList DirQueue;
			QStringList FileQueue;
		};
	}

	NewProjectWin::NewProjectWin() :QWidget() {
		this->setMinimumSize(900, 600);
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon.png"));
		this->setWindowTitle(VITRL("YSS::project.createNewProject"));
		TitleLabel = new Visindigo::Widgets::BorderLabel(this);
		TitleLabel->setObjectName("ProgramTitleLabel");
		TitleLabel->setFixedHeight(80);
		auto font = TitleLabel->font();
		font.setPointSizeF(font.pointSizeF() * 2);
		TitleLabel->setFont(font);
		RecentTemplateTitle = new Visindigo::Widgets::BorderLabel(this);
		SearchLineEdit = new QLineEdit(this);

		ProjectTemplateArea = new QScrollArea(this);
		ProjectTemplateArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
		ProjectTemplateWidget = new QWidget(ProjectTemplateArea);
		ProjectTemplateWidget->setObjectName("ProjectTemplateWidget");
		ProjectTemplateWidget->setStyleSheet("QWidget#ProjectTemplateWidget { background: transparent; }");
		ProjectTemplateArea->setWidget(ProjectTemplateWidget);
		ProjectTemplateArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		ProjectTemplateArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		ProjectTemplateLayout = new QVBoxLayout(ProjectTemplateWidget);
		ProjectTemplateWidget->setLayout(ProjectTemplateLayout);
		ProjectTemplateLayout->setContentsMargins(0, 0, 0, 0);
		ProjectTemplateLayout->setSpacing(0);

		RecentTemplateArea = new QScrollArea(this);
		RecentTemplateWidget = new QWidget(RecentTemplateArea);
		RecentTemplateArea->setWidget(RecentTemplateWidget);
		RecentTemplateLayout = new QVBoxLayout(RecentTemplateWidget);
		RecentTemplateWidget->setLayout(RecentTemplateLayout);

		Layout = new QGridLayout(this);
		Layout->addWidget(TitleLabel, 0, 0, 1, 2);
		Layout->addWidget(SearchLineEdit, 1, 0, 1, 1);
		Layout->addWidget(ProjectTemplateArea, 2, 0, 1, 1);
		Layout->addWidget(RecentTemplateTitle, 1, 1, 1, 1);
		Layout->addWidget(RecentTemplateArea, 2, 1, 1, 1);

		Layout->setSpacing(0);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setColumnStretch(0, 3);
		Layout->setColumnStretch(1, 2);

		TitleLabel->setText(" " + VITRL("YSS::project.createFromTemplate"));
		SearchLineEdit->setPlaceholderText(VITRL("YSS::project.searchWithTags"));
		SearchLineEdit->setFixedHeight(30);
		RecentTemplateTitle->setText(VITRL("YSS::project.recentUsed"));

		setColorfulEnable(true);
		onThemeChanged();
		this->loadProjectTemplate();
	}

	void NewProjectWin::loadProjectTemplate() {
		for (Visindigo::Widgets::MultiButton* button : ProjectList) {
			ProjectTemplateLayout->removeWidget(button);
			ProviderMap.remove(button);
			button->deleteLater();
		}
		ProjectList.clear();
		QList<YSSCore::Editor::ProjectTemplateProvider*> providers = YSSPTM->getProviders();
		for (YSSCore::Editor::ProjectTemplateProvider* provider : providers) {
			yDebugF << provider->getTemplateID();
			yDebugF << provider->getTemplateName();
			yDebugF << provider->getTemplateDescription();
			Visindigo::Widgets::MultiButton* ProviderButton = new Visindigo::Widgets::MultiButton(ProjectTemplateWidget);
			ProviderButton->setTitle(provider->getTemplateName());
			ProviderButton->setDescription(provider->getTemplateDescription());
			ProviderButton->setPixmapPath(provider->getTemplateIconPath());
			QLabel* tags = new QLabel(ProviderButton);
			tags->setText(provider->getTemplateTags().join("; "));
			ProviderButton->setCustomWidgetOrientation(Qt::Vertical);
			ProviderButton->addCustomWidget(tags);
			ProjectList.append(ProviderButton);
			ProjectTemplateLayout->addWidget(ProviderButton);
			ProviderMap.insert(ProviderButton, provider);
			ProviderButton->setSpacing(5);
			ProviderButton->setContentsMargins(10, 10, 10, 10);
			ProviderButton->show();
			yDebugF << ProviderButton->isHidden();
			connect(ProviderButton, &Visindigo::Widgets::MultiButton::clicked, this,
				&NewProjectWin::onTemplateButtonClicked);
		}
		ProjectTemplateWidget->setFixedHeight(providers.size() * 100);
	}
	void NewProjectWin::onTemplateButtonClicked() {
		Visindigo::Widgets::MultiButton* Button = static_cast<Visindigo::Widgets::MultiButton*>(sender());
		if (Button == nullptr) {
			return;
		}
		YSSCore::Editor::ProjectTemplateProvider* provider = ProviderMap[Button];
		if (provider == nullptr) {
			return;
		}
		PendingProvider = provider;
		ProjectLocateWin* locateWin = new ProjectLocateWin();
		locateWin->setAttribute(Qt::WA_DeleteOnClose);
		locateWin->setWindowModality(Qt::ApplicationModal);
		connect(locateWin, &ProjectLocateWin::targetChosen, this, &NewProjectWin::onLocateTargetChosen);
		locateWin->show();
	}

	void NewProjectWin::onLocateTargetChosen(const YSSCore::General::YSSURI& projectUri) {
		if (PendingProvider == nullptr) {
			return;
		}
		YSSCore::Editor::ProjectTemplateInitWidget* initWidget =
			PendingProvider->projectInitWidget(projectUri);
		if (initWidget == nullptr) {
			return;
		}
		initWidget->setAttribute(Qt::WA_DeleteOnClose);
		connect(initWidget, &YSSCore::Editor::ProjectTemplateInitWidget::projectPrepared,
			this, &NewProjectWin::onProjectPrepared);
		initWidget->setWindowModality(Qt::ApplicationModal);
		initWidget->show();
	}

	void NewProjectWin::onProjectPrepared() {
		YSSCore::Editor::ProjectTemplateInitWidget* initWidget =
			qobject_cast<YSSCore::Editor::ProjectTemplateInitWidget*>(sender());
		if (initWidget == nullptr) {
			return;
		}
		ProjectCreationJob* job = new ProjectCreationJob(initWidget->getTargetUri(),
			initWidget->getAllFileContents(), initWidget->getYSSProjectContent(),
			initWidget->getDefaultOpenFiles(), this);
		job->OnSucceeded = [this, job]() {
			const YSSCore::General::YSSURI uri = job->getProjectConfigUri();
			emit projectPrepared(uri);
			this->close();
			};
		job->OnFailed = [this](const QString& message, bool alreadyExists) {
			if (alreadyExists) {
				QMessageBox::warning(this, VITRL("YSS::project.create.exists.title"),
					VITRL("YSS::project.create.exists.message").arg(message));
			}
			else {
				QMessageBox::warning(this, VITRL("YSS::project.create.failed.title"),
					VITRL("YSS::project.create.failed.message").arg(message));
			}
			};
		job->start();
	}
	void NewProjectWin::onThemeChanged() {
		for (Visindigo::Widgets::MultiButton* button : ProjectList) {

		}
	}
	void NewProjectWin::closeEvent(QCloseEvent* event) {
		this->deleteLater();
	}
	void NewProjectWin::resizeEvent(QResizeEvent* event) {
		ProjectTemplateWidget->setFixedWidth(ProjectTemplateArea->width() - 12);
	}
}