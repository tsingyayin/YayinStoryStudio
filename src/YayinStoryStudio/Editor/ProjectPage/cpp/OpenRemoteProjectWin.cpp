#include <QtGui/qpalette.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <General/TranslationHost.h>
#include <Widgets/ThemeManager.h>
#include "Editor/SimpleWidget/RemoteBrowserWidget.h"
#include "Editor/ProjectPage/OpenRemoteProjectWin.h"

namespace YSS::ProjectPage {
	OpenRemoteProjectWin::OpenRemoteProjectWin(QWidget* parent)
		: QDialog(parent) {
		buildUi();
		applyStyle();
	}

	OpenRemoteProjectWin::~OpenRemoteProjectWin() {
	}

	void OpenRemoteProjectWin::buildUi() {
		setWindowTitle(VITRL("YSS::project.openRemote.title"));
		setMinimumWidth(720);
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(20, 18, 20, 16);
		layout->setSpacing(12);

		TitleLabel = new QLabel(VITRL("YSS::project.openRemote.title"), this);
		TitleLabel->setObjectName("remoteOpenTitleLabel");
		SubtitleLabel = new QLabel(VITRL("YSS::project.openRemote.subtitle"), this);
		SubtitleLabel->setObjectName("remoteOpenSubtitleLabel");
		SubtitleLabel->setWordWrap(true);
		layout->addWidget(TitleLabel);
		layout->addWidget(SubtitleLabel);

		auto* card = new QFrame(this);
		card->setObjectName("remoteOpenCard");
		auto* cardLayout = new QVBoxLayout(card);
		cardLayout->setContentsMargins(14, 14, 14, 14);
		cardLayout->setSpacing(10);
		layout->addWidget(card);

		RemoteBrowser = new YSS::Editor::RemoteBrowserWidget(card);
		RemoteBrowser->setFileSuffixFilters(QStringList({
			QStringLiteral(".yssp"), QStringLiteral("yssproj.json")
			}));
		cardLayout->addWidget(RemoteBrowser);

		SelectionLabel = new QLabel(this);
		SelectionLabel->setObjectName("remoteOpenSelectionLabel");
		SelectionLabel->setWordWrap(true);
		layout->addWidget(SelectionLabel);

		auto* buttonRow = new QHBoxLayout();
		buttonRow->addStretch(1);
		CancelButton = new QPushButton(VITRL("Visindigo::general.cancel"), this);
		OpenButton = new QPushButton(VITRL("YSS::project.openRemote.open"), this);
		OpenButton->setObjectName("remoteOpenConfirmButton");
		OpenButton->setDefault(true);
		buttonRow->addWidget(CancelButton);
		buttonRow->addWidget(OpenButton);
		layout->addLayout(buttonRow);

		connect(RemoteBrowser, &YSS::Editor::RemoteBrowserWidget::selectionChanged, this,
			&OpenRemoteProjectWin::updateSelectionLabel);
		connect(RemoteBrowser, &YSS::Editor::RemoteBrowserWidget::currentUriChanged, this,
			[this](const YSSCore::General::YSSURI&) { updateSelectionLabel(); });
		connect(RemoteBrowser, &YSS::Editor::RemoteBrowserWidget::fileActivated, this,
			&OpenRemoteProjectWin::chooseProject);
		connect(CancelButton, &QPushButton::clicked, this, &QDialog::close);
		connect(OpenButton, &QPushButton::clicked, this, &OpenRemoteProjectWin::onOpenClicked);
		updateSelectionLabel();
	}

	void OpenRemoteProjectWin::applyStyle() {
		setStyleSheet(VISTMGT("YSS::ProjectPage.OpenRemoteProject", this));
	}

	void OpenRemoteProjectWin::showEvent(QShowEvent* event) {
		QDialog::showEvent(event);
		RemoteBrowser->refreshAuthentics();
		RemoteBrowser->refresh();
	}

	void OpenRemoteProjectWin::updateSelectionLabel() {
		const YSSCore::General::YSSURI uri = RemoteBrowser->getSelectedFileUri();
		if (uri.isValid()) {
			SelectionLabel->setText(VITRL("YSS::project.openRemote.selected").arg(uri.toString()));
			OpenButton->setEnabled(true);
			return;
		}
		SelectionLabel->setText(VITRL("YSS::project.openRemote.none"));
		OpenButton->setEnabled(false);
	}

	void OpenRemoteProjectWin::chooseProject(const YSSCore::General::YSSURI& uri) {
		if (!uri.isValid()) {
			return;
		}
		emit projectChosen(uri);
		close();
	}

	void OpenRemoteProjectWin::onOpenClicked() {
		chooseProject(RemoteBrowser->getSelectedFileUri());
	}
}
