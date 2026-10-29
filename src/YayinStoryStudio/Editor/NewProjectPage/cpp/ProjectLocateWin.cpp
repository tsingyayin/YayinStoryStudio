#include <QtCore/qdir.h>
#include <QtGui/qpalette.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/TranslationHost.h>
#include <Utility/FileUtility.h>
#include "Editor/SimpleWidget/RemoteBrowserWidget.h"
#include "Editor/NewProjectPage/ProjectLocateWin.h"

namespace YSS::NewProjectPage {
	namespace {
		QString displayablePath(const YSSCore::General::YSSURI& uri) {
			const QString localPath = uri.getLocalPath();
			return localPath.isEmpty() ? uri.toString() : localPath;
		}
	}

	ProjectLocateWin::ProjectLocateWin(QWidget* parent)
		: QDialog(parent) {
		buildUi();
		applyStyle();
		onModeChanged();
	}

	ProjectLocateWin::~ProjectLocateWin() {
	}

	void ProjectLocateWin::buildUi() {
		setWindowTitle(VITRL("YSS::project.locate.title"));
		setMinimumWidth(720);
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(20, 18, 20, 16);
		layout->setSpacing(12);

		TitleLabel = new QLabel(VITRL("YSS::project.locate.title"), this);
		TitleLabel->setObjectName("locateTitleLabel");
		SubtitleLabel = new QLabel(VITRL("YSS::project.locate.subtitle"), this);
		SubtitleLabel->setObjectName("locateSubtitleLabel");
		SubtitleLabel->setWordWrap(true);
		layout->addWidget(TitleLabel);
		layout->addWidget(SubtitleLabel);

		auto* card = new QFrame(this);
		card->setObjectName("locateCard");
		auto* cardLayout = new QVBoxLayout(card);
		cardLayout->setContentsMargins(14, 14, 14, 14);
		cardLayout->setSpacing(10);
		layout->addWidget(card);

		auto* nameCaption = new QLabel(VITRL("YSS::project.locate.name.title"), card);
		nameCaption->setObjectName("locateCaption");
		NameEdit = new QLineEdit(card);
		NameEdit->setPlaceholderText(VITRL("YSS::project.locate.name.text"));
		cardLayout->addWidget(nameCaption);
		cardLayout->addWidget(NameEdit);

		auto* modeRow = new QHBoxLayout();
		modeRow->setSpacing(6);
		LocalModeButton = new QToolButton(card);
		LocalModeButton->setText(VITRL("YSS::project.locate.local"));
		LocalModeButton->setCheckable(true);
		LocalModeButton->setAutoExclusive(true);
		LocalModeButton->setObjectName("locateModeButton");
		LocalModeButton->setProperty("segment", "left");
		RemoteModeButton = new QToolButton(card);
		RemoteModeButton->setText(VITRL("YSS::project.locate.remote"));
		RemoteModeButton->setCheckable(true);
		RemoteModeButton->setAutoExclusive(true);
		RemoteModeButton->setObjectName("locateModeButton");
		RemoteModeButton->setProperty("segment", "right");
		modeRow->addWidget(LocalModeButton);
		modeRow->addWidget(RemoteModeButton);
		modeRow->addStretch(1);
		cardLayout->addLayout(modeRow);

		TargetStack = new QStackedWidget(card);
		cardLayout->addWidget(TargetStack);

		auto* localPage = new QWidget(TargetStack);
		auto* localLayout = new QVBoxLayout(localPage);
		localLayout->setContentsMargins(0, 0, 0, 0);
		localLayout->setSpacing(6);
		auto* localRow = new QHBoxLayout();
		localRow->setSpacing(6);
		LocalPathEdit = new QLineEdit(localPage);
		LocalPathEdit->setText(QDir::toNativeSeparators(QDir::currentPath() + QStringLiteral("/user_data/repos")));
		BrowseButton = new QPushButton(VITRL("YSS::project.locate.browse"), localPage);
		localRow->addWidget(LocalPathEdit, 1);
		localRow->addWidget(BrowseButton);
		localLayout->addLayout(localRow);
		TargetStack->addWidget(localPage);

		RemoteBrowser = new YSS::Editor::RemoteBrowserWidget(TargetStack);
		TargetStack->addWidget(RemoteBrowser);

		WhereLabel = new QLabel(this);
		WhereLabel->setObjectName("locateWhereLabel");
		WhereLabel->setWordWrap(true);
		layout->addWidget(WhereLabel);

		auto* buttonRow = new QHBoxLayout();
		buttonRow->addStretch(1);
		CancelButton = new QPushButton(VITRL("Visindigo::general.cancel"), this);
		ConfirmButton = new QPushButton(VITRL("YSS::project.locate.confirm"), this);
		ConfirmButton->setObjectName("locateConfirmButton");
		ConfirmButton->setDefault(true);
		buttonRow->addWidget(CancelButton);
		buttonRow->addWidget(ConfirmButton);
		layout->addLayout(buttonRow);

		connect(LocalModeButton, &QToolButton::clicked, this, &ProjectLocateWin::onModeChanged);
		connect(RemoteModeButton, &QToolButton::clicked, this, &ProjectLocateWin::onModeChanged);
		connect(BrowseButton, &QPushButton::clicked, this, &ProjectLocateWin::onBrowseClicked);
		connect(NameEdit, &QLineEdit::textChanged, this, &ProjectLocateWin::updateWhereLabel);
		connect(LocalPathEdit, &QLineEdit::textChanged, this, &ProjectLocateWin::updateWhereLabel);
		connect(RemoteBrowser, &YSS::Editor::RemoteBrowserWidget::currentUriChanged, this,
			[this](const YSSCore::General::YSSURI&) { updateWhereLabel(); });
		connect(CancelButton, &QPushButton::clicked, this, &QDialog::close);
		connect(ConfirmButton, &QPushButton::clicked, this, &ProjectLocateWin::onConfirmClicked);
		LocalModeButton->setChecked(true);
		updateWhereLabel();
	}

	void ProjectLocateWin::applyStyle() {
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
QLabel#locateTitleLabel { color: @text; font-size: 20px; font-weight: 600; }
QLabel#locateSubtitleLabel { color: @dim; }
QLabel#locateCaption { color: @dim; font-size: 12px; }
QLabel#locateWhereLabel { color: @dim; }

QFrame#locateCard { background: @card; border: 1px solid @border; border-radius: 10px; }

QLineEdit {
	background: @inset;
	border: 1px solid @border;
	border-radius: 6px;
	padding: 6px 10px;
	min-height: 22px;
	color: @text;
}
QLineEdit:focus { border: 1px solid @accent; }

QToolButton#locateModeButton {
	background: transparent;
	border: 1px solid @border;
	color: @dim;
	padding: 6px 16px;
}
QToolButton#locateModeButton[segment="left"] { border-top-left-radius: 6px; border-bottom-left-radius: 6px; }
QToolButton#locateModeButton[segment="right"] {
	border-top-right-radius: 6px; border-bottom-right-radius: 6px;
	border-left: none;
}
QToolButton#locateModeButton:checked {
	background: @accentFill;
	border-color: @accentFill;
	color: @onAccent;
}

QPushButton {
	background: @inset;
	border: 1px solid @border;
	border-radius: 6px;
	color: @text;
	padding: 6px 16px;
}
QPushButton:hover { border-color: @accent; }
QPushButton:disabled { color: @dim; }
QPushButton#locateConfirmButton {
	background: @accentFill;
	border-color: @accentFill;
	color: @onAccent;
	font-weight: 600;
}
QPushButton#locateConfirmButton:disabled { background: @inset; border-color: @border; color: @dim; }
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

	void ProjectLocateWin::setProjectName(const QString& name) {
		NameEdit->setText(name);
	}

	bool ProjectLocateWin::isRemoteMode() const {
		return RemoteModeButton->isChecked();
	}

	void ProjectLocateWin::showEvent(QShowEvent* event) {
		QDialog::showEvent(event);
		RemoteBrowser->refreshAuthentics();
		if (isRemoteMode()) {
			RemoteBrowser->refresh();
		}
	}

	void ProjectLocateWin::onModeChanged() {
		const bool remote = RemoteModeButton->isChecked();
		TargetStack->setCurrentIndex(remote ? 1 : 0);
		if (remote) {
			RemoteBrowser->refreshAuthentics();
			RemoteBrowser->refresh();
		}
		updateWhereLabel();
	}

	void ProjectLocateWin::onBrowseClicked() {
		const QString start = LocalPathEdit->text().trimmed();
		const QString dir = QFileDialog::getExistingDirectory(this,
			VITRL("YSS::project.locate.browse"), start);
		if (dir.isEmpty()) {
			return;
		}
		LocalPathEdit->setText(QDir::toNativeSeparators(dir));
		updateWhereLabel();
	}

	void ProjectLocateWin::updateWhereLabel() {
		const QString name = Visindigo::Utility::FileUtility::toLegalFileName(NameEdit->text()).trimmed();
		const YSSCore::General::YSSURI folder = getTargetFolder();
		if (name.isEmpty() or !folder.isValid()) {
			WhereLabel->clear();
			ConfirmButton->setEnabled(false);
			return;
		}
		WhereLabel->setText(VITRL("YSS::project.locate.where").arg(displayablePath(folder.resolve(name))));
		ConfirmButton->setEnabled(true);
	}

	void ProjectLocateWin::onConfirmClicked() {
		const QString name = Visindigo::Utility::FileUtility::toLegalFileName(NameEdit->text()).trimmed();
		const YSSCore::General::YSSURI folder = getTargetFolder();
		if (name.isEmpty() or !folder.isValid()) {
			return;
		}
		emit targetChosen(folder.resolve(name));
		close();
	}

	YSSCore::General::YSSURI ProjectLocateWin::getTargetFolder() const {
		if (RemoteModeButton->isChecked()) {
			return RemoteBrowser->getCurrentUri();
		}
		const QString path = LocalPathEdit->text().trimmed();
		if (path.isEmpty()) {
			return YSSCore::General::YSSURI();
		}
		return YSSCore::General::YSSURI::fromLocalPath(path);
	}
}
