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

namespace YSS::NewFilePage {
	NewFileWin::NewFileWin(const QString& initPath) :QWidget() {
		this->InitPath = initPath;
		this->setMinimumSize(900, 600);
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		this->setWindowTitle(YSSTR("YSS::project.createNewFile"));
		TitleLabel = new QLabel(this);
		TitleLabel->setObjectName("TitleLabel");
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

		TitleLabel->setText(" " + YSSTR("YSS::project.createFromTemplate"));
		SearchLineEdit->setPlaceholderText(YSSTR("YSS::project.searchWithTags"));
		SearchLineEdit->setFixedHeight(30);
		RecentTemplateTitle->setText(YSSTR("YSS::project.recentUsed"));

		this->setStyleSheet(YSSTMSS("YSS::ProjectWin", this));

		this->loadFileTemplate();
	}

	void NewFileWin::loadFileTemplate() {
		for (YSSCore::Widgets::MultiButton* button : FileList) {
			FileTemplateLayout->removeWidget(button);
			ProviderMap.remove(button);
			button->deleteLater();
		}
		FileList.clear();
		QList<YSSCore::Editor::FileTemplateProvider*> providers = YSSFTM->getProviders();
		for (YSSCore::Editor::FileTemplateProvider* provider : providers) {
			yDebug << provider->getTemplateID();
			yDebug << provider->getTemplateName();
			yDebug << provider->getTemplateDescription();
			YSSCore::Widgets::MultiButton* ProviderButton = new YSSCore::Widgets::MultiButton(FileTemplateWidget);
			ProviderButton->setTitle(provider->getTemplateName());
			ProviderButton->setDescription(provider->getTemplateDescription());
			ProviderButton->setPixmapPath(provider->getTemplateIconPath());
			ProviderButton->setNormalStyleSheet(YSSTMSS("YSS::FileWin.HistoryFile.Normal"));
			ProviderButton->setHoverStyleSheet(YSSTMSS("YSS::FileWin.HistoryFile.Hover"));
			ProviderButton->setPressedStyleSheet(YSSTMSS("YSS::FileWin.HistoryFile.Pressed"));
			QLabel* tags = new QLabel(ProviderButton);
			tags->setText(provider->getTemplateTags().join("; "));
			ProviderButton->addCustomWidget(tags);
			FileList.append(ProviderButton);
			FileTemplateLayout->addWidget(ProviderButton);
			ProviderMap.insert(ProviderButton, provider);
			ProviderButton->setSpacing(0);
			ProviderButton->setContentsMargins(5, 5, 5, 5);
			ProviderButton->show();
			yDebug << ProviderButton->isHidden();
			connect(ProviderButton, &YSSCore::Widgets::MultiButton::clicked, this,
				&NewFileWin::onTemplateButtonClicked);
		}
		FileTemplateWidget->setFixedHeight(providers.size() * 90);
	}
	void NewFileWin::onTemplateButtonClicked() {
		YSSCore::Widgets::MultiButton* Button = static_cast<YSSCore::Widgets::MultiButton*>(sender());
		if (Button == nullptr) {
			return;
		}
		YSSCore::Editor::FileTemplateProvider* provider = ProviderMap[Button];
		YSSCore::Editor::FileTemplateInitWidget* initWidget = provider->fileInitWidget(InitPath);
		if (initWidget != nullptr) {
			initWidget->setAttribute(Qt::WA_DeleteOnClose);
			connect(initWidget, &YSSCore::Editor::FileTemplateInitWidget::closed,
				this, &NewFileWin::onTemplateInitWidgetClosed);
			connect(initWidget, &YSSCore::Editor::FileTemplateInitWidget::filePrepared,
				this, &NewFileWin::onFilePrepared);
			initWidget->setWindowModality(Qt::ApplicationModal);
			initWidget->show();
		}
	}
	void NewFileWin::closeEvent(QCloseEvent* event) {
	}
	void NewFileWin::resizeEvent(QResizeEvent* event) {
		FileTemplateWidget->setFixedWidth(FileTemplateArea->width() - FileTemplateArea->verticalScrollBar()->width());
	}
	void NewFileWin::onTemplateInitWidgetClosed() {
	}
	void NewFileWin::onFilePrepared(QString projectPath) {
		emit filePrepared(projectPath);
		this->close();
	}
}