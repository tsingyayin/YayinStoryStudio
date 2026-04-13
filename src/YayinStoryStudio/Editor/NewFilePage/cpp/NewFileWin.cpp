#include "../NewFileWin.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>

#include <Widgets/ThemeManager.h>
#include <General/TranslationHost.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/FileTemplateProvider.h>
#include <Widgets/MultiButton.h>
#include <QtCore/qlist.h>
#include <QtGui/qicon.h>
#include <General/Log.h>
#include <General/YSSLogger.h>

namespace YSS::NewFilePage {
	NewFileWin::NewFileWin(const QString& initPath) :QWidget() {
		this->InitPath = initPath;
		this->setMinimumSize(900, 600);
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/yssicon.png"));
		this->setWindowTitle(VITR("YSS::project.createNewFile"));
		TitleLabel = new QLabel(this);
		TitleLabel->setObjectName("ProgramTitleLabel");
		TitleLabel->setFixedHeight(80);
		RecentTemplateTitle = new QLabel(this);
		SearchLineEdit = new QLineEdit(this);

		FileTemplateArea = new QScrollArea(this);
		FileTemplateWidget = new QWidget(FileTemplateArea);
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

		TitleLabel->setText(" " + VITR("YSS::project.createFromTemplate"));
		SearchLineEdit->setPlaceholderText(VITR("YSS::project.searchWithTags"));
		SearchLineEdit->setFixedHeight(30);
		RecentTemplateTitle->setText(VITR("YSS::project.recentUsed"));

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
			ProviderButton->setNormalStyleSheet(VISTMGT("YSS::General.MultiButton.Normal"));
			ProviderButton->setHoverStyleSheet(VISTMGT("YSS::General.MultiButton.Hover"));
			ProviderButton->setPressedStyleSheet(VISTMGT("YSS::General.MultiButton.Pressed"));
			QLabel* tags = new QLabel(ProviderButton);
			tags->setText(provider->getTemplateTags().join("; "));
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
		YSSCore::Editor::FileTemplateInitWidget* initWidget = provider->fileInitWidget(InitPath);
		if (initWidget != nullptr) {
			initWidget->setAttribute(Qt::WA_DeleteOnClose);
			connect(initWidget, &YSSCore::Editor::FileTemplateInitWidget::filePrepared,
				this, &NewFileWin::onFilePrepared);
			initWidget->setWindowModality(Qt::ApplicationModal);
			initWidget->show();
		}
	}
	void NewFileWin::onThemeChanged() {
		this->applyVIStyleTemplate("YSS::ProjectWin");
		for (Visindigo::Widgets::MultiButton* button : FileList) {
			button->setNormalStyleSheet(VISTMGT("YSS::General.MultiButton.Normal"));
			button->setHoverStyleSheet(VISTMGT("YSS::General.MultiButton.Hover"));
			button->setPressedStyleSheet(VISTMGT("YSS::General.MultiButton.Pressed"));
		}
	}
	void NewFileWin::closeEvent(QCloseEvent* event) {
		this->deleteLater();
	}
	void NewFileWin::resizeEvent(QResizeEvent* event) {
		FileTemplateWidget->setFixedWidth(FileTemplateArea->width() - FileTemplateArea->verticalScrollBar()->width());
	}
	void NewFileWin::onFilePrepared(QString projectPath) {
		emit filePrepared(projectPath);
		this->close();
	}
}