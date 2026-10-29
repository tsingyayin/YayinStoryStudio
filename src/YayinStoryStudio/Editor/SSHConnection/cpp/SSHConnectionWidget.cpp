#include <QtCore/qfileinfo.h>
#include <QtGui/qfontdatabase.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/Plugin.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <General/YSSUniqueFileSystem.h>
#include <Utility/JsonConfig.h>
#include "Editor/SSHConnection/SSHConnectionWidget.h"
namespace YSS::Editor {
	namespace {
		SSHConnectionWidget* Instance = nullptr;
		constexpr int MaxAuthenticCount = 20;
		QString authenticsKey() { return QStringLiteral("SSH.Authentics"); }
		QString hostKeysKey() { return QStringLiteral("SSH.HostKeys"); }
		QString authorityField() { return QStringLiteral("Authority"); }
		QString keyTypeField() { return QStringLiteral("KeyType"); }
		QString fingerprintField() { return QStringLiteral("FingerprintSha256"); }
	}
	SSHConnectionWidget* SSHConnectionWidget::getInstance() {
		if (Instance == nullptr) {
			Instance = new SSHConnectionWidget();
		}
		return Instance;
	}

	void SSHConnectionWidget::requestVerification(const YSSCore::General::YSSURI& uri) {
		getInstance()->requestVerificationFor(uri);
	}

	SSHConnectionWidget::SSHConnectionWidget(QWidget* parent)
		:QDialog(parent) {
		Instance = this;
		this->buildUi();
		YSSCore::General::YSSUniqueFileSystem* fs =
			YSSCore::General::YSSUniqueFileSystem::getInstance();
		connect(fs, &YSSCore::General::YSSUniqueFileSystem::sshHostKeyAccepted,
			this, &SSHConnectionWidget::onHostKeyAccepted);
		connect(fs, &YSSCore::General::YSSUniqueFileSystem::sshHostKeyRejected,
			this, &SSHConnectionWidget::onHostKeyRejected);
		connect(fs, &YSSCore::General::YSSUniqueFileSystem::sshConnectFinished,
			this, &SSHConnectionWidget::onConnectFinished);
		connect(fs, &YSSCore::General::YSSUniqueFileSystem::sshReadyChanged,
			this, &SSHConnectionWidget::onSshReadyChanged);
		this->refreshAuthenticBox();
		this->refreshConnectionList();
	}

	SSHConnectionWidget::~SSHConnectionWidget() {
		if (Instance == this) {
			Instance = nullptr;
		}
	}

	void SSHConnectionWidget::buildUi() {
		this->setWindowTitle(VITRL("YSS::sshConnection.title"));
		this->setMinimumWidth(560);
		Layout = new QVBoxLayout(this);
		Layout->setContentsMargins(20, 18, 20, 16);
		Layout->setSpacing(12);
		this->setLayout(Layout);

		TitleLabel = new QLabel(VITRL("YSS::sshConnection.title"), this);
		TitleLabel->setObjectName(QStringLiteral("sshTitleLabel"));
		Layout->addWidget(TitleLabel);
		SubtitleLabel = new QLabel(VITRL("YSS::sshConnection.subtitle"), this);
		SubtitleLabel->setObjectName(QStringLiteral("sshSubtitleLabel"));
		SubtitleLabel->setWordWrap(true);
		Layout->addWidget(SubtitleLabel);

		QFrame* credentialCard = new QFrame(this);
		credentialCard->setObjectName(QStringLiteral("sshCard"));
		QVBoxLayout* credentialLayout = new QVBoxLayout(credentialCard);
		credentialLayout->setContentsMargins(16, 14, 16, 16);
		credentialLayout->setSpacing(8);
		credentialCard->setLayout(credentialLayout);
		Layout->addWidget(credentialCard);

		QLabel* authenticCaption = new QLabel(VITRL("YSS::sshConnection.authentic"), credentialCard);
		authenticCaption->setObjectName(QStringLiteral("sshCaption"));
		credentialLayout->addWidget(authenticCaption);

		AuthenticBox = new QComboBox(credentialCard);
		AuthenticBox->setObjectName(QStringLiteral("sshAuthenticBox"));
		AuthenticBox->setEditable(true);
		AuthenticBox->setInsertPolicy(QComboBox::NoInsert);
		if (AuthenticBox->lineEdit() != nullptr) {
			AuthenticBox->lineEdit()->setPlaceholderText(QStringLiteral("user@host:port"));
		}
		credentialLayout->addWidget(AuthenticBox);

		// 凭据方式做成分段控件，比两个单选按钮紧凑，也贴近现在的界面习惯。
		QFrame* modeSegment = new QFrame(credentialCard);
		modeSegment->setObjectName(QStringLiteral("sshSegment"));
		QHBoxLayout* modeLayout = new QHBoxLayout(modeSegment);
		modeLayout->setContentsMargins(3, 3, 3, 3);
		modeLayout->setSpacing(3);
		modeSegment->setLayout(modeLayout);
		PasswordModeButton = new QToolButton(modeSegment);
		PasswordModeButton->setObjectName(QStringLiteral("sshPasswordModeButton"));
		PasswordModeButton->setText(VITRL("YSS::sshConnection.credential.password"));
		KeyModeButton = new QToolButton(modeSegment);
		KeyModeButton->setObjectName(QStringLiteral("sshKeyModeButton"));
		KeyModeButton->setText(VITRL("YSS::sshConnection.credential.key"));
		QToolButton* modeButtons[] = { PasswordModeButton, KeyModeButton };
		for (QToolButton* button : modeButtons) {
			button->setCheckable(true);
			button->setAutoExclusive(true);
			button->setToolButtonStyle(Qt::ToolButtonTextOnly);
			button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			button->setCursor(Qt::PointingHandCursor);
			modeLayout->addWidget(button, 1);
		}
		PasswordModeButton->setChecked(true);
		credentialLayout->addWidget(modeSegment);

		CredentialStack = new QStackedWidget(credentialCard);
		// 不给凭据区留纵向余量：多余高度应该给下面的连接列表，而不是把输入框拉高。
		CredentialStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		PasswordEdit = new QLineEdit(CredentialStack);
		PasswordEdit->setObjectName(QStringLiteral("sshPasswordEdit"));
		PasswordEdit->setEchoMode(QLineEdit::Password);
		PasswordEdit->setPlaceholderText(VITRL("YSS::sshConnection.credential.passwordHint"));
		CredentialStack->addWidget(PasswordEdit);

		QWidget* keyPage = new QWidget(CredentialStack);
		QHBoxLayout* keyLayout = new QHBoxLayout(keyPage);
		keyLayout->setContentsMargins(0, 0, 0, 0);
		keyLayout->setSpacing(8);
		KeyPathEdit = new QLineEdit(keyPage);
		KeyPathEdit->setObjectName(QStringLiteral("sshKeyPathEdit"));
		KeyPathEdit->setPlaceholderText(VITRL("YSS::sshConnection.credential.keyHint"));
		BrowseKeyButton = new QPushButton(VITRL("Visindigo::general.select"), keyPage);
		BrowseKeyButton->setObjectName(QStringLiteral("sshBrowseKeyButton"));
		BrowseKeyButton->setProperty("sshRole", "secondary");
		keyLayout->addWidget(KeyPathEdit, 1);
		keyLayout->addWidget(BrowseKeyButton);
		CredentialStack->addWidget(keyPage);
		credentialLayout->addWidget(CredentialStack);

		QHBoxLayout* connectLayout = new QHBoxLayout();
		connectLayout->setSpacing(12);
		ConnectButton = new QPushButton(VITRL("YSS::sshConnection.connect"), credentialCard);
		ConnectButton->setObjectName(QStringLiteral("sshConnectButton"));
		ConnectButton->setProperty("sshRole", "primary");
		ConnectButton->setDefault(true);
		ConnectButton->setCursor(Qt::PointingHandCursor);
		ConnectButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		connectLayout->addWidget(ConnectButton);

		StatusLabel = new QLabel(credentialCard);
		StatusLabel->setObjectName(QStringLiteral("sshStatusLabel"));
		StatusLabel->setWordWrap(true);
		StatusLabel->hide();
		connectLayout->addWidget(StatusLabel, 1);
		connectLayout->addStretch();
		credentialLayout->addLayout(connectLayout);

		FingerprintPanel = new QFrame(this);
		FingerprintPanel->setObjectName(QStringLiteral("sshFingerprintPanel"));
		FingerprintPanel->setFrameShape(QFrame::NoFrame);
		QVBoxLayout* fingerprintLayout = new QVBoxLayout(FingerprintPanel);
		fingerprintLayout->setContentsMargins(16, 14, 16, 14);
		fingerprintLayout->setSpacing(10);
		FingerprintPanel->setLayout(fingerprintLayout);
		FingerprintLabel = new QLabel(FingerprintPanel);
		FingerprintLabel->setObjectName(QStringLiteral("sshFingerprintLabel"));
		FingerprintLabel->setWordWrap(true);
		FingerprintLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
		FingerprintLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
		fingerprintLayout->addWidget(FingerprintLabel);
		QHBoxLayout* fingerprintButtonLayout = new QHBoxLayout();
		fingerprintButtonLayout->setSpacing(8);
		TrustFingerprintButton = new QPushButton(
			VITRL("YSS::sshConnection.fingerprint.trust"), FingerprintPanel);
		TrustFingerprintButton->setObjectName(QStringLiteral("sshTrustFingerprintButton"));
		TrustFingerprintButton->setProperty("sshRole", "primary");
		TrustFingerprintButton->setCursor(Qt::PointingHandCursor);
		RejectFingerprintButton = new QPushButton(
			VITRL("YSS::sshConnection.fingerprint.reject"), FingerprintPanel);
		RejectFingerprintButton->setObjectName(QStringLiteral("sshRejectFingerprintButton"));
		RejectFingerprintButton->setProperty("sshRole", "secondary");
		fingerprintButtonLayout->addWidget(TrustFingerprintButton);
		fingerprintButtonLayout->addWidget(RejectFingerprintButton);
		fingerprintButtonLayout->addStretch();
		fingerprintLayout->addLayout(fingerprintButtonLayout);
		FingerprintPanel->hide();
		Layout->addWidget(FingerprintPanel);

		QFrame* listCard = new QFrame(this);
		listCard->setObjectName(QStringLiteral("sshCard"));
		QVBoxLayout* listLayout = new QVBoxLayout(listCard);
		listLayout->setContentsMargins(16, 14, 16, 16);
		listLayout->setSpacing(10);
		listCard->setLayout(listLayout);
		Layout->addWidget(listCard, 1);

		QHBoxLayout* listHeaderLayout = new QHBoxLayout();
		listHeaderLayout->setSpacing(8);
		QLabel* listLabel = new QLabel(VITRL("YSS::sshConnection.list.title"), listCard);
		listLabel->setObjectName(QStringLiteral("sshCaption"));
		RefreshButton = new QPushButton(VITRL("Visindigo::general.refresh"), listCard);
		RefreshButton->setObjectName(QStringLiteral("sshRefreshButton"));
		RefreshButton->setProperty("sshRole", "ghost");
		RefreshButton->setCursor(Qt::PointingHandCursor);
		listHeaderLayout->addWidget(listLabel);
		listHeaderLayout->addStretch();
		listHeaderLayout->addWidget(RefreshButton);
		listLayout->addLayout(listHeaderLayout);

		ConnectionList = new QListWidget(listCard);
		ConnectionList->setObjectName(QStringLiteral("sshConnectionList"));
		ConnectionList->setSelectionMode(QAbstractItemView::SingleSelection);
		ConnectionList->setMinimumHeight(150);
		ConnectionList->setFrameShape(QFrame::NoFrame);
		ConnectionList->setIconSize(QSize(16, 16));
		ConnectionList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
		listLayout->addWidget(ConnectionList, 1);

		QHBoxLayout* listButtonLayout = new QHBoxLayout();
		listButtonLayout->setSpacing(8);
		DisconnectButton = new QPushButton(VITRL("YSS::sshConnection.disconnect"), listCard);
		DisconnectButton->setObjectName(QStringLiteral("sshDisconnectButton"));
		DisconnectButton->setProperty("sshRole", "secondary");
		DisconnectButton->setCursor(Qt::PointingHandCursor);
		RemoveButton = new QPushButton(VITRL("YSS::sshConnection.remove"), listCard);
		RemoveButton->setObjectName(QStringLiteral("sshRemoveButton"));
		RemoveButton->setProperty("sshRole", "danger");
		RemoveButton->setCursor(Qt::PointingHandCursor);
		listButtonLayout->addWidget(DisconnectButton);
		listButtonLayout->addWidget(RemoveButton);
		listButtonLayout->addStretch();
		listLayout->addLayout(listButtonLayout);

		QHBoxLayout* bottomLayout = new QHBoxLayout();
		bottomLayout->addStretch();
		QPushButton* closeButton = new QPushButton(VITRL("Visindigo::general.close"), this);
		closeButton->setObjectName(QStringLiteral("sshCloseButton"));
		closeButton->setProperty("sshRole", "ghost");
		closeButton->setCursor(Qt::PointingHandCursor);
		bottomLayout->addWidget(closeButton);
		Layout->addLayout(bottomLayout);

		connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
		connect(ConnectButton, &QPushButton::clicked, this, &SSHConnectionWidget::onConnectClicked);
		connect(BrowseKeyButton, &QPushButton::clicked, this, &SSHConnectionWidget::onBrowseKeyClicked);
		connect(PasswordModeButton, &QToolButton::toggled,
			this, &SSHConnectionWidget::onCredentialModeChanged);
		connect(KeyModeButton, &QToolButton::toggled,
			this, &SSHConnectionWidget::onCredentialModeChanged);
		connect(DisconnectButton, &QPushButton::clicked,
			this, &SSHConnectionWidget::onDisconnectClicked);
		connect(RemoveButton, &QPushButton::clicked, this, &SSHConnectionWidget::onRemoveClicked);
		connect(RefreshButton, &QPushButton::clicked,
			this, &SSHConnectionWidget::refreshConnectionList);
		connect(TrustFingerprintButton, &QPushButton::clicked,
			this, &SSHConnectionWidget::onTrustFingerprintClicked);
		connect(RejectFingerprintButton, &QPushButton::clicked,
			this, &SSHConnectionWidget::onRejectFingerprintClicked);
		// QSS 的 :disabled 伪状态和动态属性选择器一起用时不可靠，这里用自定义属性
		// 显式驱动"不可用"外观，同时保留 setEnabled 控制交互。
		const auto setActionMuted = [](QPushButton* button, bool muted) {
			button->setEnabled(!muted);
			if (button->property("sshMuted").toBool() == muted) {
				return;
			}
			button->setProperty("sshMuted", muted);
			button->style()->unpolish(button);
			button->style()->polish(button);
			};
		connect(ConnectionList, &QListWidget::itemSelectionChanged, this,
			[this, setActionMuted]() {
				// 用选中项而不是当前项：列表首次填充时 Qt 会自动把第一项设为当前项，
				// 那不算用户选中。
				const bool muted = this->ConnectionList->selectedItems().isEmpty();
				setActionMuted(this->DisconnectButton, muted);
				setActionMuted(this->RemoveButton, muted);
			});
		if (AuthenticBox->lineEdit() != nullptr) {
			connect(AuthenticBox->lineEdit(), &QLineEdit::returnPressed,
				this, &SSHConnectionWidget::onConnectClicked);
		}
		connect(PasswordEdit, &QLineEdit::returnPressed,
			this, &SSHConnectionWidget::onConnectClicked);
		connect(KeyPathEdit, &QLineEdit::returnPressed,
			this, &SSHConnectionWidget::onConnectClicked);
		this->DisconnectButton->setProperty("sshMuted", true);
		this->RemoveButton->setProperty("sshMuted", true);
		this->DisconnectButton->setEnabled(false);
		this->RemoveButton->setEnabled(false);
		this->resize(680, 640);
	}

	void SSHConnectionWidget::showEvent(QShowEvent* event) {
		// 对话框是单例，存活期间主题可能被切换，每次显示时按当前配色重建样式。
		this->applyStyle();
		QDialog::showEvent(event);
	}

	void SSHConnectionWidget::applyStyle() {
		// 样式只从控件自身的调色板推导：程序的明暗切换走的是 Qt 调色板（setColorScheme），
		// 不依赖 ThemeManager 里的配色方案键值。
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
		// 调色板里的 PlaceholderText 往往带 alpha（name() 会丢掉透明度，直接取色等于拿到了
		// 正文色），所以这里按正文色向卡片底色混合，自己算一个真正偏弱的次级文字色。
		const auto blend = [](const QColor& from, const QColor& to, qreal ratio) {
			return QColor::fromRgbF(
				from.redF() * (1.0 - ratio) + to.redF() * ratio,
				from.greenF() * (1.0 - ratio) + to.greenF() * ratio,
				from.blueF() * (1.0 - ratio) + to.blueF() * ratio);
			};
		const QColor dim = blend(text, cardBg, 0.45);
		// 深色主题下高亮色偏亮，铺白字对比度不够，填充前压暗一点。
		const QColor accentFill = lightTheme ? accent : accent.darker(118);
		const QColor danger = lightTheme ? QColor(0xC0, 0x27, 0x1C) : QColor(0xFF, 0x6F, 0x61);
		const QColor success = lightTheme ? QColor(0x12, 0x8A, 0x21) : QColor(0x3D, 0xDC, 0x63);
		const auto rgba = [](const QColor& color, int alpha) {
			return QStringLiteral("rgba(%1, %2, %3, %4)")
				.arg(color.red()).arg(color.green()).arg(color.blue()).arg(alpha);
			};
		QString qss = QStringLiteral(R"QSS(
QLabel#sshTitleLabel { color: @text; font-size: 20px; font-weight: 600; }
QLabel#sshSubtitleLabel { color: @dim; }
QLabel#sshCaption { color: @dim; font-size: 12px; }
QLabel#sshStatusLabel { color: @dim; }
QLabel#sshStatusLabel[sshError="true"] { color: @danger; }

QFrame#sshCard { background: @card; border: 1px solid @border; border-radius: 10px; }

QComboBox#sshAuthenticBox, QLineEdit#sshPasswordEdit, QLineEdit#sshKeyPathEdit {
	background: @inset;
	border: 1px solid @border;
	border-radius: 6px;
	padding: 6px 10px;
	min-height: 22px;
	color: @text;
}
QComboBox#sshAuthenticBox:focus, QLineEdit#sshPasswordEdit:focus, QLineEdit#sshKeyPathEdit:focus {
	border: 1px solid @accent;
}
QComboBox#sshAuthenticBox QLineEdit { background: transparent; border: none; padding: 0; }
QComboBox#sshAuthenticBox::drop-down { border: none; width: 20px; }
QComboBox#sshAuthenticBox QAbstractItemView {
	background: @card;
	border: 1px solid @border;
	color: @text;
	selection-background-color: @accent;
	selection-color: @onAccent;
}

QFrame#sshSegment { background: @inset; border: 1px solid @border; border-radius: 8px; }
QToolButton#sshPasswordModeButton, QToolButton#sshKeyModeButton {
	background: transparent;
	border: none;
	border-radius: 6px;
	padding: 5px 12px;
	min-height: 22px;
	color: @dim;
}
QToolButton#sshPasswordModeButton:hover, QToolButton#sshKeyModeButton:hover { color: @text; }
QToolButton#sshPasswordModeButton:checked, QToolButton#sshKeyModeButton:checked {
	background: @accent;
	color: @onAccent;
	font-weight: 600;
}

QPushButton[sshRole="primary"] {
	background: @accent;
	color: @onAccent;
	border: none;
	border-radius: 6px;
	padding: 8px 24px;
	min-height: 24px;
	font-weight: 600;
}
QPushButton[sshRole="primary"]:hover { background: @accentHover; }
QPushButton[sshRole="primary"]:pressed { background: @accentPressed; }

QPushButton[sshRole="secondary"], QPushButton[sshRole="ghost"] {
	background: transparent;
	border: 1px solid @border;
	border-radius: 6px;
	padding: 6px 14px;
	min-height: 22px;
	color: @text;
}
QPushButton[sshRole="secondary"]:hover, QPushButton[sshRole="ghost"]:hover { background: @inset; }

QPushButton[sshRole="danger"] {
	background: transparent;
	border: 1px solid @border;
	border-radius: 6px;
	padding: 6px 14px;
	min-height: 22px;
	color: @danger;
}
QPushButton[sshRole="danger"]:hover { border: 1px solid @danger; }

QPushButton[sshMuted="true"] {
	background: transparent;
	border: 1px solid @border;
	color: @dim;
}

QFrame#sshFingerprintPanel {
	background: @warnBg;
	border: 1px solid @warnBorder;
	border-radius: 10px;
}
QLabel#sshFingerprintLabel { color: @text; }

QListWidget#sshConnectionList { background: transparent; border: none; outline: none; }
QListWidget#sshConnectionList::item { border-radius: 8px; margin: 2px 0; }
QListWidget#sshConnectionList::item:hover { background: @inset; }
QListWidget#sshConnectionList::item:selected { background: @selection; }

QLabel#sshRowTitle { color: @text; font-weight: 600; }
QLabel#sshRowStateOn { color: @success; font-size: 12px; }
QLabel#sshRowStateOff { color: @dim; font-size: 12px; }
QLabel#sshRowDotOn { color: @success; }
QLabel#sshRowDotOff { color: @dim; }
QLabel#sshEmptyHint { color: @dim; }
)QSS");
		qss.replace(QStringLiteral("@text"), text.name());
		qss.replace(QStringLiteral("@dim"), dim.name());
		qss.replace(QStringLiteral("@onAccent"), onAccent.name());
		qss.replace(QStringLiteral("@accentHover"), accentFill.lighter(112).name());
		qss.replace(QStringLiteral("@accentPressed"), accentFill.darker(112).name());
		qss.replace(QStringLiteral("@accent"), accentFill.name());
		qss.replace(QStringLiteral("@card"), cardBg.name());
		qss.replace(QStringLiteral("@inset"), insetBg.name());
		qss.replace(QStringLiteral("@border"), border.name());
		qss.replace(QStringLiteral("@danger"), danger.name());
		qss.replace(QStringLiteral("@success"), success.name());
		qss.replace(QStringLiteral("@warnBg"), rgba(danger, 32));
		qss.replace(QStringLiteral("@warnBorder"), rgba(danger, 140));
		qss.replace(QStringLiteral("@selection"), rgba(accent, 70));
		this->setStyleSheet(qss);
	}

	QWidget* SSHConnectionWidget::createConnectionRow(const QString& authority, bool ready) const {
		QWidget* row = new QWidget();
		row->setObjectName(QStringLiteral("sshConnectionRow"));
		row->setAttribute(Qt::WA_TransparentForMouseEvents);
		QHBoxLayout* layout = new QHBoxLayout(row);
		layout->setContentsMargins(10, 7, 10, 7);
		layout->setSpacing(10);
		QLabel* dot = new QLabel(row);
		dot->setObjectName(ready ? QStringLiteral("sshRowDotOn") : QStringLiteral("sshRowDotOff"));
		dot->setText(QStringLiteral("\u25CF"));
		dot->setAttribute(Qt::WA_TransparentForMouseEvents);
		QLabel* title = new QLabel(authority, row);
		title->setObjectName(QStringLiteral("sshRowTitle"));
		title->setAttribute(Qt::WA_TransparentForMouseEvents);
		QLabel* state = new QLabel(row);
		state->setObjectName(ready ? QStringLiteral("sshRowStateOn") : QStringLiteral("sshRowStateOff"));
		state->setText(ready
			? VITRL("YSS::sshConnection.state.ready")
			: VITRL("YSS::sshConnection.state.idle"));
		state->setAttribute(Qt::WA_TransparentForMouseEvents);
		layout->addWidget(dot);
		layout->addWidget(title, 1);
		layout->addWidget(state);
		return row;
	}

	QWidget* SSHConnectionWidget::createEmptyRow() const {
		QWidget* row = new QWidget();
		row->setAttribute(Qt::WA_TransparentForMouseEvents);
		QHBoxLayout* layout = new QHBoxLayout(row);
		layout->setContentsMargins(10, 12, 10, 12);
		QLabel* hint = new QLabel(VITRL("YSS::sshConnection.list.empty"), row);
		hint->setObjectName(QStringLiteral("sshEmptyHint"));
		hint->setAttribute(Qt::WA_TransparentForMouseEvents);
		layout->addWidget(hint);
		layout->addStretch();
		return row;
	}

	void SSHConnectionWidget::refreshAuthenticBox() {
		const QString current = AuthenticBox->currentText();
		AuthenticBox->clear();
		AuthenticBox->addItems(SSHConnectionWidget::loadAuthentics());
		if (!current.isEmpty()) {
			AuthenticBox->setEditText(current);
		}
	}

	void SSHConnectionWidget::refreshConnectionList() {
		YSSCore::General::YSSUniqueFileSystem* fs =
			YSSCore::General::YSSUniqueFileSystem::getInstance();
		QListWidgetItem* previous = ConnectionList->currentItem();
		const QString previousAuthority = previous == nullptr
			? QString()
			: previous->data(Qt::UserRole).toString();
		ConnectionList->clear();
		// 数据源合并三份：本次会话登记过的连接（可直接连）、历史 authentic、已确认指纹的主机。
		QStringList authorities;
		const QStringList registered = fs->getSshAuthorities();
		for (const QString& authority : registered) {
			if (!authorities.contains(authority)) {
				authorities.append(authority);
			}
		}
		const QStringList authentics = SSHConnectionWidget::loadAuthentics();
		for (const QString& authority : authentics) {
			if (!authorities.contains(authority)) {
				authorities.append(authority);
			}
		}
		const QStringList knownHosts = SSHConnectionWidget::loadKnownHosts();
		for (const QString& authority : knownHosts) {
			if (!authorities.contains(authority)) {
				authorities.append(authority);
			}
		}
		for (const QString& authority : authorities) {
			const bool ready = fs->isSshReady(authority);
			QListWidgetItem* item = new QListWidgetItem(ConnectionList);
			item->setData(Qt::UserRole, authority);
			item->setSizeHint(QSize(0, 52));
			ConnectionList->setItemWidget(item, this->createConnectionRow(authority, ready));
			if (!previousAuthority.isEmpty() && authority == previousAuthority) {
				ConnectionList->setCurrentItem(item);
				item->setSelected(true);
			}
		}
		if (ConnectionList->count() == 0) {
			QListWidgetItem* item = new QListWidgetItem(ConnectionList);
			item->setFlags(Qt::NoItemFlags);
			item->setSizeHint(QSize(0, 44));
			ConnectionList->setItemWidget(item, this->createEmptyRow());
		}
	}

	void SSHConnectionWidget::requestVerificationFor(const YSSCore::General::YSSURI& uri) {
		if (uri.getScheme() != YSSCore::General::YSSURI::Scheme::SSH) {
			return;
		}
		const std::optional<Visindigo::Network::SshPath> path = uri.getSshPath();
		if (!path.has_value()) {
			return;
		}
		const QString authority = path->getAuthority();
		if (authority.isEmpty()) {
			return;
		}
		this->refreshAuthenticBox();
		this->refreshConnectionList();
		this->hideFingerprintPrompt();
		this->setBusy(false);
		AuthenticBox->setEditText(authority);
		PasswordEdit->clear();
		this->setStatus(VITRL("YSS::sshConnection.status.needCredential").arg(authority));
		this->show();
		this->raise();
		this->activateWindow();
		if (PasswordModeButton->isChecked()) {
			PasswordEdit->setFocus();
		}
		else {
			KeyPathEdit->setFocus();
		}
	}

	QString SSHConnectionWidget::currentAuthentic() const {
		return AuthenticBox->currentText().trimmed();
	}

	void SSHConnectionWidget::onConnectClicked() {
		const QString authority = this->currentAuthentic();
		QString user;
		QString host;
		qint32 port = 22;
		if (!SSHConnectionWidget::parseAuthority(authority, &user, &host, &port)) {
			this->setStatus(VITRL("YSS::sshConnection.status.invalidAuthentic"), true);
			return;
		}
		if (PasswordModeButton->isChecked()) {
			if (PasswordEdit->text().isEmpty()) {
				this->setStatus(VITRL("YSS::sshConnection.status.needPassword"), true);
				return;
			}
		}
		else {
			const QString keyPath = KeyPathEdit->text().trimmed();
			if (keyPath.isEmpty()) {
				this->setStatus(VITRL("YSS::sshConnection.status.needKeyFile"), true);
				return;
			}
			if (!QFileInfo::exists(keyPath)) {
				this->setStatus(
					VITRL("YSS::sshConnection.status.keyFileMissing").arg(keyPath), true);
				return;
			}
		}
		this->connectTo(authority);
	}

	void SSHConnectionWidget::connectTo(const QString& authority) {
		QString user;
		QString host;
		qint32 port = 22;
		if (!SSHConnectionWidget::parseAuthority(authority, &user, &host, &port)) {
			this->setStatus(VITRL("YSS::sshConnection.status.invalidAuthentic"), true);
			return;
		}
		YSSCore::General::YSSUniqueFileSystem* fs =
			YSSCore::General::YSSUniqueFileSystem::getInstance();
		if (fs->isSshReady(authority)) {
			this->setStatus(VITRL("YSS::sshConnection.status.alreadyReady").arg(authority));
			this->refreshConnectionList();
			emit this->connectionReady(authority);
			return;
		}
		Visindigo::Network::SshConnectConfig config;
		config.Host = host;
		config.Port = port;
		config.User = user;
		if (KeyModeButton->isChecked()) {
			config.AuthMethod = Visindigo::Network::SshAuthMethod::PublicKey;
			config.PrivateKeyPath = KeyPathEdit->text().trimmed();
		}
		else {
			config.AuthMethod = Visindigo::Network::SshAuthMethod::Password;
			config.Password = PasswordEdit->text();
		}
		// 指纹由本页按主插件配置里的记录自行比对，所以这里放开 libssh2 的 known_hosts
		// 检查。未经确认的主机会在 hostKeyAccepted 里立刻断开，不会在其上做任何文件操作。
		config.HostKeyPolicy = Visindigo::Network::SshHostKeyPolicy::Insecure;
		config.ConnectTimeoutMs = 15000;
		if (!fs->addSshConnection(config)) {
			this->setStatus(VITRL("YSS::sshConnection.status.configRejected").arg(authority), true);
			return;
		}
		PendingAuthority = authority;
		this->setBusy(true);
		this->setStatus(VITRL("YSS::sshConnection.status.connecting").arg(authority));
		this->refreshConnectionList();
		fs->connectSsh(authority);
	}

	void SSHConnectionWidget::onBrowseKeyClicked() {
		const QString path = QFileDialog::getOpenFileName(this,
			VITRL("YSS::sshConnection.credential.keyHint"));
		if (path.isEmpty()) {
			return;
		}
		KeyPathEdit->setText(path);
	}

	void SSHConnectionWidget::onCredentialModeChanged() {
		CredentialStack->setCurrentIndex(KeyModeButton->isChecked() ? 1 : 0);
	}

	void SSHConnectionWidget::onDisconnectClicked() {
		QListWidgetItem* item = ConnectionList->currentItem();
		if (item == nullptr) {
			return;
		}
		const QString authority = item->data(Qt::UserRole).toString();
		if (authority.isEmpty()) {
			return;
		}
		YSSCore::General::YSSUniqueFileSystem::getInstance()->disconnectSsh(authority);
		this->setStatus(VITRL("YSS::sshConnection.status.disconnected").arg(authority));
		this->refreshConnectionList();
	}

	void SSHConnectionWidget::onRemoveClicked() {
		QListWidgetItem* item = ConnectionList->currentItem();
		if (item == nullptr) {
			return;
		}
		const QString authority = item->data(Qt::UserRole).toString();
		if (authority.isEmpty()) {
			return;
		}
		YSSCore::General::YSSUniqueFileSystem::getInstance()->removeSshConnection(authority);
		SSHConnectionWidget::forgetFingerprint(authority);
		SSHConnectionWidget::forgetAuthentic(authority);
		this->refreshAuthenticBox();
		this->setStatus(VITRL("YSS::sshConnection.status.removed").arg(authority));
		this->refreshConnectionList();
	}

	void SSHConnectionWidget::showFingerprintPrompt(const QString& authority,
		const Visindigo::Network::SshHostKeyInfo& info) {
		const QString stored = SSHConnectionWidget::findStoredFingerprint(authority);
		const QString prompt = stored.isEmpty()
			? VITRL("YSS::sshConnection.fingerprint.firstTime")
			: VITRL("YSS::sshConnection.fingerprint.changed");
		// 指纹与密钥类型直接拼在提示后面，避免翻译缺失时用户看不到要确认的内容。
		QString fingerprint = info.FingerprintSha256;
		if (!fingerprint.startsWith(QStringLiteral("SHA256:"), Qt::CaseInsensitive)) {
			fingerprint.prepend(QStringLiteral("SHA256:"));
		}
		FingerprintLabel->setText(QStringLiteral("%1\n\n[%2]\n%3  %4")
			.arg(prompt, authority, info.KeyType, fingerprint));
		FingerprintPanel->show();
	}

	void SSHConnectionWidget::hideFingerprintPrompt() {
		FingerprintPanel->hide();
		FingerprintLabel->clear();
	}

	void SSHConnectionWidget::onTrustFingerprintClicked() {
		const QString authority = AwaitingFingerprint;
		if (authority.isEmpty()) {
			return;
		}
		SSHConnectionWidget::storeFingerprint(authority, PendingHostKey);
		this->hideFingerprintPrompt();
		AwaitingFingerprint.clear();
		PendingHostKey = Visindigo::Network::SshHostKeyInfo();
		this->setStatus(VITRL("YSS::sshConnection.status.fingerprintTrusted").arg(authority));
		this->connectTo(authority);
	}

	void SSHConnectionWidget::onRejectFingerprintClicked() {
		const QString authority = AwaitingFingerprint;
		this->hideFingerprintPrompt();
		AwaitingFingerprint.clear();
		PendingHostKey = Visindigo::Network::SshHostKeyInfo();
		if (!authority.isEmpty()) {
			this->setStatus(
				VITRL("YSS::sshConnection.status.fingerprintRejected").arg(authority), true);
		}
		this->refreshConnectionList();
	}

	void SSHConnectionWidget::onHostKeyAccepted(const QString& authority,
		const Visindigo::Network::SshHostKeyInfo& info) {
		if (!AwaitingFingerprint.isEmpty()) {
			return;
		}
		const QString stored = SSHConnectionWidget::findStoredFingerprint(authority);
		if (!stored.isEmpty()
			&& stored.compare(info.FingerprintSha256, Qt::CaseInsensitive) == 0) {
			return;
		}
		// 未记录或与记录不符：先断开，避免在未确认的主机上继续使用连接。
		AwaitingFingerprint = authority;
		PendingHostKey = info;
		YSSCore::General::YSSUniqueFileSystem::getInstance()->disconnectSsh(authority);
		this->setBusy(false);
		this->showFingerprintPrompt(authority, info);
		this->setStatus(VITRL("YSS::sshConnection.status.needFingerprint").arg(authority));
		this->refreshConnectionList();
	}

	void SSHConnectionWidget::onHostKeyRejected(const QString& authority,
		const Visindigo::Network::SshHostKeyInfo& info) {
		this->setStatus(VITRL("YSS::sshConnection.status.hostKeyRejected")
			.arg(authority, info.KeyType, info.FingerprintSha256), true);
		this->refreshConnectionList();
	}

	void SSHConnectionWidget::onConnectFinished(const QString& authority, bool ok,
		const QString& message) {
		if (!AwaitingFingerprint.isEmpty() && AwaitingFingerprint == authority) {
			return;
		}
		if (PendingAuthority == authority) {
			PendingAuthority.clear();
			this->setBusy(false);
		}
		if (ok) {
			SSHConnectionWidget::rememberAuthentic(authority);
			this->refreshAuthenticBox();
			this->setStatus(VITRL("YSS::sshConnection.status.connected").arg(authority));
			emit this->connectionReady(authority);
		}
		else {
			this->setStatus(message.isEmpty()
				? VITRL("YSS::sshConnection.status.connectFailed").arg(authority)
				: message, true);
		}
		this->refreshConnectionList();
	}

	void SSHConnectionWidget::onSshReadyChanged(const QString&, bool) {
		this->refreshConnectionList();
	}

	void SSHConnectionWidget::setStatus(const QString& text, bool error) {
		StatusLabel->setVisible(!text.isEmpty());
		StatusLabel->setText(text);
		if (StatusLabel->property("sshError").toBool() != error) {
			StatusLabel->setProperty("sshError", error);
			StatusLabel->style()->unpolish(StatusLabel);
			StatusLabel->style()->polish(StatusLabel);
		}
		StatusLabel->setToolTip(text);
	}

	void SSHConnectionWidget::setBusy(bool busy) {
		ConnectButton->setEnabled(!busy);
		if (ConnectButton->property("sshMuted").toBool() != busy) {
			ConnectButton->setProperty("sshMuted", busy);
			ConnectButton->style()->unpolish(ConnectButton);
			ConnectButton->style()->polish(ConnectButton);
		}
		AuthenticBox->setEnabled(!busy);
		PasswordModeButton->setEnabled(!busy);
		KeyModeButton->setEnabled(!busy);
		CredentialStack->setEnabled(!busy);
	}

	QStringList SSHConnectionWidget::loadAuthentics() {
		Visindigo::General::Plugin* plugin = VIApp->getMainPlugin();
		if (plugin == nullptr) {
			return QStringList();
		}
		Visindigo::Utility::JsonConfig* config = plugin->getPluginConfig();
		if (config == nullptr) {
			return QStringList();
		}
		QStringList authentics = config->getStringList(authenticsKey());
		authentics.removeAll(QString());
		return authentics;
	}

	void SSHConnectionWidget::rememberAuthentic(const QString& authority) {
		if (authority.isEmpty()) {
			return;
		}
		Visindigo::General::Plugin* plugin = VIApp->getMainPlugin();
		if (plugin == nullptr) {
			return;
		}
		Visindigo::Utility::JsonConfig* config = plugin->getPluginConfig();
		if (config == nullptr) {
			return;
		}
		QStringList authentics = config->getStringList(authenticsKey());
		authentics.removeAll(authority);
		authentics.prepend(authority);
		while (authentics.size() > MaxAuthenticCount) {
			authentics.removeLast();
		}
		config->setStringList(authenticsKey(), authentics);
		plugin->savePluginConfig();
	}

	void SSHConnectionWidget::forgetAuthentic(const QString& authority) {
		if (authority.isEmpty()) {
			return;
		}
		Visindigo::General::Plugin* plugin = VIApp->getMainPlugin();
		if (plugin == nullptr) {
			return;
		}
		Visindigo::Utility::JsonConfig* config = plugin->getPluginConfig();
		if (config == nullptr) {
			return;
		}
		QStringList authentics = config->getStringList(authenticsKey());
		if (authentics.removeAll(authority) == 0) {
			return;
		}
		config->setStringList(authenticsKey(), authentics);
		plugin->savePluginConfig();
	}

	QStringList SSHConnectionWidget::loadKnownHosts() {
		Visindigo::General::Plugin* plugin = VIApp->getMainPlugin();
		if (plugin == nullptr) {
			return QStringList();
		}
		Visindigo::Utility::JsonConfig* config = plugin->getPluginConfig();
		if (config == nullptr) {
			return QStringList();
		}
		QStringList authorities;
		const QList<Visindigo::Utility::JsonConfig> entries = config->getArray(hostKeysKey());
		for (const Visindigo::Utility::JsonConfig& entry : entries) {
			const QString authority = entry.getString(authorityField());
			if (authority.isEmpty() || authorities.contains(authority)) {
				continue;
			}
			authorities.append(authority);
		}
		return authorities;
	}

	QString SSHConnectionWidget::findStoredFingerprint(const QString& authority) {
		Visindigo::General::Plugin* plugin = VIApp->getMainPlugin();
		if (plugin == nullptr) {
			return QString();
		}
		Visindigo::Utility::JsonConfig* config = plugin->getPluginConfig();
		if (config == nullptr) {
			return QString();
		}
		const QList<Visindigo::Utility::JsonConfig> entries = config->getArray(hostKeysKey());
		for (const Visindigo::Utility::JsonConfig& entry : entries) {
			if (entry.getString(authorityField()).compare(authority, Qt::CaseInsensitive) == 0) {
				return entry.getString(fingerprintField());
			}
		}
		return QString();
	}

	void SSHConnectionWidget::storeFingerprint(const QString& authority,
		const Visindigo::Network::SshHostKeyInfo& info) {
		if (authority.isEmpty() || info.FingerprintSha256.isEmpty()) {
			return;
		}
		Visindigo::General::Plugin* plugin = VIApp->getMainPlugin();
		if (plugin == nullptr) {
			return;
		}
		Visindigo::Utility::JsonConfig* config = plugin->getPluginConfig();
		if (config == nullptr) {
			return;
		}
		QList<Visindigo::Utility::JsonConfig> entries = config->getArray(hostKeysKey());
		bool replaced = false;
		for (Visindigo::Utility::JsonConfig& entry : entries) {
			if (entry.getString(authorityField()).compare(authority, Qt::CaseInsensitive) != 0) {
				continue;
			}
			entry.setString(keyTypeField(), info.KeyType);
			entry.setString(fingerprintField(), info.FingerprintSha256);
			replaced = true;
			break;
		}
		if (!replaced) {
			Visindigo::Utility::JsonConfig entry;
			entry.setString(authorityField(), authority);
			entry.setString(keyTypeField(), info.KeyType);
			entry.setString(fingerprintField(), info.FingerprintSha256);
			entries.append(entry);
		}
		config->setArray(hostKeysKey(), entries);
		plugin->savePluginConfig();
	}

	void SSHConnectionWidget::forgetFingerprint(const QString& authority) {
		Visindigo::General::Plugin* plugin = VIApp->getMainPlugin();
		if (plugin == nullptr) {
			return;
		}
		Visindigo::Utility::JsonConfig* config = plugin->getPluginConfig();
		if (config == nullptr) {
			return;
		}
		QList<Visindigo::Utility::JsonConfig> entries = config->getArray(hostKeysKey());
		bool removed = false;
		for (int i = entries.size() - 1; i >= 0; --i) {
			if (entries.at(i).getString(authorityField()).compare(
				authority, Qt::CaseInsensitive) == 0) {
				entries.removeAt(i);
				removed = true;
			}
		}
		if (!removed) {
			return;
		}
		config->setArray(hostKeysKey(), entries);
		plugin->savePluginConfig();
	}

	bool SSHConnectionWidget::parseAuthority(const QString& authority, QString* user,
		QString* host, qint32* port) {
		const QString trimmed = authority.trimmed();
		const qsizetype at = trimmed.lastIndexOf(QLatin1Char('@'));
		if (at <= 0 || at >= trimmed.size() - 1) {
			return false;
		}
		QString hostPart = trimmed.mid(at + 1);
		qint32 portNumber = 22;
		const qsizetype colon = hostPart.lastIndexOf(QLatin1Char(':'));
		if (colon >= 0) {
			bool ok = false;
			const qint32 parsed = hostPart.mid(colon + 1).toInt(&ok);
			if (!ok || parsed < 1 || parsed > 65535) {
				return false;
			}
			portNumber = parsed;
			hostPart = hostPart.left(colon);
		}
		if (hostPart.isEmpty()) {
			return false;
		}
		*user = trimmed.left(at);
		*host = hostPart;
		*port = portNumber;
		return true;
	}
}
