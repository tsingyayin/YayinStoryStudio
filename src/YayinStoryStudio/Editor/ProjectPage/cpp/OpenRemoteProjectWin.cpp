#include <QtGui/qpalette.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <General/TranslationHost.h>
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
		const QPalette pal = this->palette();
		const QColor window = pal.color(QPalette::Window);
		const QColor base = pal.color(QPalette::Base);
		const QColor text = pal.color(QPalette::WindowText);
		QColor accent = pal.color(QPalette::Highlight);
		if (!accent.isValid()) {
			accent = pal.color(QPalette::Accent);
		}
		const QColor onAccent = pal.color(QPalette::HighlightedText);
		const bool lightTheme = window.lightness() > 128;
		const QColor cardBg = lightTheme ? base : window.lighter(115);
		const QColor insetBg = lightTheme ? window.darker(104) : base;
		const QColor border = lightTheme ? window.darker(120) : window.lighter(140);
		const auto blend = [](const QColor& from, const QColor& to, qreal ratio) {
			return QColor::fromRgbF(
				from.redF() * (1.0 - ratio) + to.redF() * ratio,
				from.greenF() * (1.0 - ratio) + to.greenF() * ratio,
				from.blueF() * (1.0 - ratio) + to.blueF() * ratio);
			};
		const QColor dim = blend(text, cardBg, 0.45);
		const QColor accentFill = lightTheme ? accent : accent.darker(118);
		QString qss = QStringLiteral(R"QSS(
QLabel#remoteOpenTitleLabel { color: @text; font-size: 20px; font-weight: 600; }
QLabel#remoteOpenSubtitleLabel { color: @dim; }
QLabel#remoteOpenSelectionLabel { color: @dim; }

QFrame#remoteOpenCard { background: @card; border: 1px solid @border; border-radius: 10px; }

QPushButton {
	background: @inset;
	border: 1px solid @border;
	border-radius: 6px;
	color: @text;
	padding: 6px 16px;
}
QPushButton:hover { border-color: @accent; }
QPushButton:disabled { color: @dim; }
QPushButton#remoteOpenConfirmButton {
	background: @accentFill;
	border-color: @accentFill;
	color: @onAccent;
	font-weight: 600;
}
QPushButton#remoteOpenConfirmButton:disabled { background: @inset; border-color: @border; color: @dim; }
)QSS");
		qss.replace(QLatin1String("@text"), text.name());
		qss.replace(QLatin1String("@dim"), dim.name());
		qss.replace(QLatin1String("@card"), cardBg.name());
		qss.replace(QLatin1String("@inset"), insetBg.name());
		qss.replace(QLatin1String("@border"), border.name());
		qss.replace(QLatin1String("@accentFill"), accentFill.name());
		qss.replace(QLatin1String("@accent"), accent.name());
		qss.replace(QLatin1String("@onAccent"), onAccent.name());
		this->setStyleSheet(qss);
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
