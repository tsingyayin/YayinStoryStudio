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
#include <Widgets/ThemeManager.h>
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
		setStyleSheet(VISTMGT("YSS::ProjectPage.ProjectLocate", this));
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
		const bool remote = RemoteModeButton->isChecked();
		if (!folder.isValid()) {
			// 远端目标目录只有在连接就绪且成功列出之后才有效，否则下一步不应该放行。
			WhereLabel->setText(remote
				? VITRL("YSS::project.locate.remoteTargetRequired") : QString());
			ConfirmButton->setEnabled(false);
			return;
		}
		if (name.isEmpty()) {
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
