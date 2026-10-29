#include <QtCore/qlist.h>
#include <QtGui/qicon.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qscrollbar.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/FileTemplateProvider.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/YSSLogger.h>
#include <General/YSSUniqueFileSystem.h>
#include <Utility/FileUtility.h>
#include <Widgets/BorderLabel.h>
#include <Widgets/MultiButton.h>
#include <Widgets/ThemeManager.h>
#include "Editor/NewFilePage/NewFileWin.h"

namespace YSS::NewFilePage {
	NewFileWin::NewFileWin(const YSSCore::General::YSSURI& initUri) :QWidget() {
		this->InitUri = initUri;
		this->setMinimumSize(900, 600);
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon.png"));
		this->setWindowTitle(VITRL("YSS::project.createNewFile"));
		TitleLabel = new Visindigo::Widgets::BorderLabel(this);
		TitleLabel->setObjectName("ProgramTitleLabel");
		TitleLabel->setFixedHeight(80);
		auto font = TitleLabel->font();
		font.setPointSizeF(font.pointSizeF() * 2);
		TitleLabel->setFont(font);
		RecentTemplateTitle = new Visindigo::Widgets::BorderLabel(this);
		SearchLineEdit = new QLineEdit(this);

		FileTemplateArea = new QScrollArea(this);
		FileTemplateArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
		FileTemplateWidget = new QWidget(FileTemplateArea);
		FileTemplateWidget->setObjectName("FileTemplateWidget");
		FileTemplateWidget->setStyleSheet("QWidget#FileTemplateWidget { background: transparent; }");
		FileTemplateArea->setWidget(FileTemplateWidget);
		FileTemplateArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		FileTemplateArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		FileTemplateLayout = new QVBoxLayout(FileTemplateWidget);
		FileTemplateWidget->setLayout(FileTemplateLayout);
		FileTemplateLayout->setContentsMargins(0, 0, 0, 0);
		FileTemplateLayout->setSpacing(0);

		RecentTemplateArea = new QScrollArea(this);
		RecentTemplateWidget = new QWidget(RecentTemplateArea);
		RecentTemplateArea->setWidget(RecentTemplateWidget);
		RecentTemplateLayout = new QVBoxLayout(RecentTemplateWidget);
		RecentTemplateWidget->setLayout(RecentTemplateLayout);

		Layout = new QGridLayout(this);
		Layout->addWidget(TitleLabel, 0, 0, 1, 2);
		Layout->addWidget(SearchLineEdit, 1, 0, 1, 1);
		Layout->addWidget(FileTemplateArea, 2, 0, 1, 1);
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

		this->loadFileTemplate();
	}

	void NewFileWin::loadFileTemplate() {
		for (Visindigo::Widgets::MultiButton* button : FileList) {
			FileTemplateLayout->removeWidget(button);
			ProviderMap.remove(button);
			button->deleteLater();
		}
		FileList.clear();
		QList<YSSCore::Editor::FileTemplateProvider*> providers = YSSFTM->getProviders();
		for (YSSCore::Editor::FileTemplateProvider* provider : providers) {
			yDebugF << provider->getTemplateID();
			yDebugF << provider->getTemplateName();
			yDebugF << provider->getTemplateDescription();
			Visindigo::Widgets::MultiButton* ProviderButton = new Visindigo::Widgets::MultiButton(FileTemplateWidget);
			ProviderButton->setTitle(provider->getTemplateName());
			ProviderButton->setDescription(provider->getTemplateDescription());
			ProviderButton->setPixmapPath(provider->getTemplateIconPath());
			QLabel* tags = new QLabel(ProviderButton);
			tags->setText(provider->getTemplateTags().join("; "));
			ProviderButton->setCustomWidgetOrientation(Qt::Vertical);
			ProviderButton->addCustomWidget(tags);
			FileList.append(ProviderButton);
			FileTemplateLayout->addWidget(ProviderButton);
			ProviderMap.insert(ProviderButton, provider);
			ProviderButton->setSpacing(5);
			ProviderButton->setContentsMargins(10, 10, 10, 10);
			ProviderButton->show();
			yDebugF << ProviderButton->isHidden();
			connect(ProviderButton, &Visindigo::Widgets::MultiButton::clicked, this,
				&NewFileWin::onTemplateButtonClicked);
		}
		FileTemplateWidget->setFixedHeight(providers.size() * 100);
	}
	void NewFileWin::onTemplateButtonClicked() {
		Visindigo::Widgets::MultiButton* Button = static_cast<Visindigo::Widgets::MultiButton*>(sender());
		if (Button == nullptr) {
			return;
		}
		YSSCore::Editor::FileTemplateProvider* provider = ProviderMap[Button];
		if (provider == nullptr) {
			return;
		}
		YSSCore::Editor::FileTemplateInitWidget* initWidget = provider->fileInitWidget(this->InitUri);
		if (initWidget == nullptr) {
			// 无界面模板：不需要任何输入，按模板名直接建一个空文件。
			QString baseName = Visindigo::Utility::FileUtility::toLegalFileName(
				provider->getTemplateName()).trimmed();
			if (baseName.isEmpty()) {
				baseName = provider->getTemplateID();
			}
			if (baseName.isEmpty()) {
				this->reportCreateFailure(VITRL("YSS::project.newFile.invalidName.title"),
					VITRL("YSS::project.newFile.invalidName.message"));
				return;
			}
			this->createFile(this->InitUri.resolve(baseName), QByteArray());
			return;
		}
		initWidget->setAttribute(Qt::WA_DeleteOnClose);
		connect(initWidget, &YSSCore::Editor::FileTemplateInitWidget::contentPrepared,
			this, [this, initWidget]() {
				const QString targetName = initWidget->getTargetName().trimmed();
				if (targetName.isEmpty() || targetName.contains(QLatin1Char('/'))
					|| targetName.contains(QLatin1Char('\\'))) {
					QMessageBox::warning(this, VITRL("YSS::project.newFile.invalidName.title"),
						VITRL("YSS::project.newFile.invalidName.message"));
					return;
				}
				this->createFile(this->InitUri.resolve(targetName), initWidget->getBinaryContent());
			});
		initWidget->setWindowModality(Qt::ApplicationModal);
		initWidget->show();
	}

	void NewFileWin::createFile(const YSSCore::General::YSSURI& uri, const QByteArray& content) {
		if (not uri.isValid()) {
			this->reportCreateFailure(VITRL("YSS::project.newFile.invalidName.title"),
				VITRL("YSS::project.newFile.invalidName.message"));
			return;
		}
		YSSCore::General::YSSUniqueFileSystem* fs =
			YSSCore::General::YSSUniqueFileSystem::getInstance();
		// 先探一次目标：存在就报错，绝不能覆盖已有文件。
		YSSCore::General::YSSFileSystemReply* statReply =
			fs->execute(uri, YSSCore::General::YSSUniqueFileSystem::Operation::Stat);
		if (statReply == nullptr) {
			this->reportCreateFailure(VITRL("YSS::project.newFile.failed.title"),
				VITRL("YSS::project.newFile.failed.message").arg(uri.toString()));
			return;
		}
		connect(statReply, &YSSCore::General::YSSFileSystemReply::succeeded, this, [this, uri]() {
			this->reportCreateFailure(VITRL("YSS::project.newFile.exists.title"),
				VITRL("YSS::project.newFile.exists.message").arg(uri.toString()));
			});
		connect(statReply, &YSSCore::General::YSSFileSystemReply::failed, this,
			[this, uri, content](const YSSCore::General::YSSFileError& error) {
				using ErrorType = YSSCore::General::YSSFileError::ErrorType;
				if (error.Type != ErrorType::NotFound) {
					this->reportCreateFailure(VITRL("YSS::project.newFile.failed.title"),
						VITRL("YSS::project.newFile.failed.message").arg(error.toString()));
					return;
				}
				YSSCore::General::YSSFileSystemReply* writeReply =
					YSSCore::General::YSSUniqueFileSystem::getInstance()->execute(
						uri, YSSCore::General::YSSUniqueFileSystem::Operation::Write, content);
				if (writeReply == nullptr) {
					this->reportCreateFailure(VITRL("YSS::project.newFile.failed.title"),
						VITRL("YSS::project.newFile.failed.message").arg(uri.toString()));
					return;
				}
				connect(writeReply, &YSSCore::General::YSSFileSystemReply::failed, this,
					[this](const YSSCore::General::YSSFileError& writeError) {
						this->reportCreateFailure(VITRL("YSS::project.newFile.failed.title"),
							VITRL("YSS::project.newFile.failed.message").arg(writeError.toString()));
					});
				connect(writeReply, &YSSCore::General::YSSFileSystemReply::succeeded, this,
					[this, uri]() {
						emit this->filePrepared(uri);
						this->close();
					});
			});
	}

	void NewFileWin::reportCreateFailure(const QString& title, const QString& message) {
		QMessageBox::warning(this, title, message);
		this->close();
	}

	void NewFileWin::onThemeChanged() {

	}
	void NewFileWin::closeEvent(QCloseEvent* event) {
		this->deleteLater();
	}
	void NewFileWin::resizeEvent(QResizeEvent* event) {
		FileTemplateWidget->setFixedWidth(FileTemplateArea->width() - 12);
	}
}