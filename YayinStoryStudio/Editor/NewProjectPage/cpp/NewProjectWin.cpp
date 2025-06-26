#include "../NewProjectWin.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>

#include <Widgets/ThemeManager.h>
#include <General/TranslationHost.h>
#include <Editor/ProjectTemplateManager.h>
#include <Editor/ProjectTemplateProvider.h>
#include <Widgets/MultiButton.h>
#include <QtCore/qlist.h>
#include <QtGui/qicon.h>
#include <General/Log.h>

namespace YSS::NewProjectPage {
	NewProjectWin::NewProjectWin() :QWidget() {
		this->setMinimumSize(900, 600);
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		this->setWindowTitle(YSSTR("YSS::project.createNewProject"));
		TitleLabel = new QLabel(this);
		TitleLabel->setObjectName("TitleLabel");
		TitleLabel->setFixedHeight(80);
		RecentTemplateTitle = new QLabel(this);
		SearchLineEdit = new QLineEdit(this);

		ProjectTemplateArea = new QScrollArea(this);
		ProjectTemplateWidget = new QWidget(ProjectTemplateArea);
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

		TitleLabel->setText(" " + YSSTR("YSS::project.createFromTemplate"));
		SearchLineEdit->setPlaceholderText(YSSTR("YSS::project.searchWithTags"));
		SearchLineEdit->setFixedHeight(30);
		RecentTemplateTitle->setText(YSSTR("YSS::project.recentUsed"));

		this->setStyleSheet(YSSTM->getStyleSheet("ProjectWin", this));

		this->loadProjectTemplate();
	}

	void NewProjectWin::loadProjectTemplate() {
		for (YSSCore::Widgets::MultiButton* button : ProjectList) {
			ProjectTemplateLayout->removeWidget(button);
			ProviderMap.remove(button);
			button->deleteLater();
		}
		ProjectList.clear();
		QList<YSSCore::Editor::ProjectTemplateProvider*> providers = YSSPTM->getProviders();
		for (YSSCore::Editor::ProjectTemplateProvider* provider : providers) {
			yDebug << provider->getTemplateID();
			yDebug << provider->getTemplateName();
			yDebug << provider->getTemplateDescription();
			YSSCore::Widgets::MultiButton* ProviderButton = new YSSCore::Widgets::MultiButton(ProjectTemplateWidget);
			ProviderButton->setTitle(provider->getTemplateName());
			ProviderButton->setDescription(provider->getTemplateDescription());
			ProviderButton->setPixmap(QPixmap::fromImage(provider->getTemplateIcon()));
			ProviderButton->setNormalStyleSheet(YSSTM->getStyleSheet("ProjectWin.HistoryProject.Normal"));
			ProviderButton->setHoverStyleSheet(YSSTM->getStyleSheet("ProjectWin.HistoryProject.Hover"));
			ProviderButton->setPressedStyleSheet(YSSTM->getStyleSheet("ProjectWin.HistoryProject.Pressed"));
			QLabel* tags = new QLabel(ProviderButton);
			tags->setText(provider->getTemplateTags().join("; "));
			ProviderButton->addCustomWidget(tags);
			ProjectList.append(ProviderButton);
			ProjectTemplateLayout->addWidget(ProviderButton);
			ProviderMap.insert(ProviderButton, provider);
			ProviderButton->setSpacing(0);
			ProviderButton->setContentsMargins(5, 5, 5, 5);
			ProviderButton->show();
			yDebug << ProviderButton->isHidden();
			connect(ProviderButton, &YSSCore::Widgets::MultiButton::clicked, this,
				&NewProjectWin::onTemplateButtonClicked);
		}
		ProjectTemplateWidget->setFixedHeight(providers.size() * 90);
	}
	void NewProjectWin::onTemplateButtonClicked() {
		YSSCore::Widgets::MultiButton* Button = static_cast<YSSCore::Widgets::MultiButton*>(sender());
		if (Button == nullptr) {
			return;
		}
		YSSCore::Editor::ProjectTemplateProvider* provider = ProviderMap[Button];
		YSSCore::Editor::ProjectTemplateInitWidget* initWidget = provider->projectInitWidget();
		if (initWidget != nullptr) {
			initWidget->setAttribute(Qt::WA_DeleteOnClose);
			connect(initWidget, &YSSCore::Editor::ProjectTemplateInitWidget::closed,
				this, &NewProjectWin::onTemplateInitWidgetClosed);
			connect(initWidget, &YSSCore::Editor::ProjectTemplateInitWidget::projectPrepared,
				this, &NewProjectWin::onProjectPrepared);
			initWidget->setWindowModality(Qt::ApplicationModal);
			initWidget->show();
		}
	}
	void NewProjectWin::closeEvent(QCloseEvent* event) {
	}
	void NewProjectWin::resizeEvent(QResizeEvent* event) {
		ProjectTemplateWidget->setFixedWidth(ProjectTemplateArea->width() - ProjectTemplateArea->verticalScrollBar()->width());
	}
	void NewProjectWin::onTemplateInitWidgetClosed() {
	}
	void NewProjectWin::onProjectPrepared(QString projectPath) {
		emit projectPrepared(projectPath);
		this->close();
	}
}