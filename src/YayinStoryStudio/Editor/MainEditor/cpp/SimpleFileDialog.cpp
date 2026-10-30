#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qregularexpression.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qstyle.h>
#include <General/TranslationHost.h>
#include <Widgets/ThemeManager.h>
#include "Editor/MainEditor/SimpleFileDialog.h"
namespace YSS::Editor {
	RenameDialog::RenameDialog() :QDialog() {
		this->setWindowTitle(VITRL("YSS::editor.renameDialog.title"));
		this->setMinimumWidth(560);
		// 路径可能很长，交给副标题换行，别让窗口被撑到屏幕外。
		this->setMaximumWidth(720);

		Layout = new QVBoxLayout(this);
		Layout->setContentsMargins(20, 18, 20, 16);
		Layout->setSpacing(12);
		this->setLayout(Layout);

		TitleLabel = new QLabel(VITRL("YSS::editor.renameDialog.title"), this);
		TitleLabel->setObjectName(QStringLiteral("renameTitleLabel"));
		Layout->addWidget(TitleLabel);

		SubtitleLabel = new QLabel(this);
		SubtitleLabel->setObjectName(QStringLiteral("renameSubtitleLabel"));
		SubtitleLabel->setWordWrap(true);
		Layout->addWidget(SubtitleLabel);

		QFrame* card = new QFrame(this);
		card->setObjectName(QStringLiteral("renameCard"));
		QVBoxLayout* cardLayout = new QVBoxLayout(card);
		cardLayout->setContentsMargins(16, 14, 16, 16);
		cardLayout->setSpacing(8);
		card->setLayout(cardLayout);
		Layout->addWidget(card);

		QLabel* nameCaption = new QLabel(VITRL("YSS::editor.renameDialog.request"), card);
		nameCaption->setObjectName(QStringLiteral("renameCaption"));
		cardLayout->addWidget(nameCaption);

		Edit = new QLineEdit(card);
		Edit->setObjectName(QStringLiteral("renameNameEdit"));
		cardLayout->addWidget(Edit);

		WarningLabel = new QLabel(card);
		WarningLabel->setObjectName(QStringLiteral("renameWarningLabel"));
		WarningLabel->setWordWrap(true);
		WarningLabel->hide();
		cardLayout->addWidget(WarningLabel);

		QHBoxLayout* buttonLayout = new QHBoxLayout();
		buttonLayout->setSpacing(8);
		CancelButton = new QPushButton(VITRL("Visindigo::general.cancel"), card);
		CancelButton->setObjectName(QStringLiteral("renameCancelButton"));
		CancelButton->setProperty("dialogRole", "secondary");
		CancelButton->setCursor(Qt::PointingHandCursor);
		ConfirmButton = new QPushButton(VITRL("Visindigo::general.confirm"), card);
		ConfirmButton->setObjectName(QStringLiteral("renameConfirmButton"));
		ConfirmButton->setProperty("dialogRole", "primary");
		ConfirmButton->setCursor(Qt::PointingHandCursor);
		ConfirmButton->setDefault(true);
		ConfirmButton->setEnabled(false);
		ConfirmButton->setProperty("renameMuted", true);
		buttonLayout->addStretch();
		buttonLayout->addWidget(CancelButton);
		buttonLayout->addWidget(ConfirmButton);
		cardLayout->addLayout(buttonLayout);
		connect(ConfirmButton, &QPushButton::clicked, this, [this]() {
			if (not validNewName) {
				return;
			}
			QString newName = Edit->text();
			if (not dirMode) {
				const QString oldExt = OldUri.getFileExt();
				const QString newExt = QFileInfo(newName).suffix();
				if (oldExt != newExt && !newExt.isEmpty()) {
					bool ok = QMessageBox::question(this, VITRL("YSS::editor.renameDialog.warning.extNotSame.title"),
						VITRL("YSS::editor.renameDialog.warning.extNotSame.message").arg(newExt).arg(oldExt),
						QMessageBox::Yes | QMessageBox::No,
						QMessageBox::No) == QMessageBox::Yes;
					if (not ok) {
						return;
					}
				}
				// 新名称没写扩展名时沿用原扩展名；原文件本来就没有扩展名则不加。
				if (newExt.isEmpty() && not oldExt.isEmpty()) {
					newName += "." + oldExt;
				}
			}
			const YSSCore::General::YSSURI newUri = targetUri(newName);
			if (not newUri.isValid()) {
				setWarning(VITRL("YSS::editor.renameDialog.warning.invalid"), true);
				return;
			}
			emit renameConfirmed(OldUri, newUri, dirMode);

			this->close();
			});

		connect(CancelButton, &QPushButton::clicked, this, [this]() {
			this->close();
			});

		connect(Edit, &QLineEdit::textChanged, this, &RenameDialog::onNewNameChanged);
		connect(Edit, &QLineEdit::returnPressed, ConfirmButton, &QPushButton::click);
	}

	void RenameDialog::showEvent(QShowEvent* event) {
		// 对话框复用同一个实例，主题可能在两次显示之间被切换，因此每次显示时重建样式。
		this->applyStyle();
		QDialog::showEvent(event);
		// 打开时整段选中原名，用户可以直接输入新名字。
		Edit->setFocus();
		Edit->selectAll();
	}

	void RenameDialog::applyStyle() {
		setStyleSheet(VISTMGT("YSS::Editor.RenameDialog", this));
	}

	void RenameDialog::setWarning(const QString& text, bool error) {
		WarningLabel->setText(text);
		WarningLabel->setVisible(not text.isEmpty());
		WarningLabel->setToolTip(text);
		if (WarningLabel->property("renameError").toBool() == error) {
			return;
		}
		WarningLabel->setProperty("renameError", error);
		WarningLabel->style()->unpolish(WarningLabel);
		WarningLabel->style()->polish(WarningLabel);
	}

	void RenameDialog::setConfirmEnabled(bool enabled) {
		ConfirmButton->setEnabled(enabled);
		// QSS 的 :disabled 伪状态和动态属性选择器一起用时不可靠，用属性显式驱动外观。
		if (ConfirmButton->property("renameMuted").toBool() == !enabled) {
			return;
		}
		ConfirmButton->setProperty("renameMuted", !enabled);
		ConfirmButton->style()->unpolish(ConfirmButton);
		ConfirmButton->style()->polish(ConfirmButton);
	}

	void RenameDialog::setContext(const YSSCore::General::YSSURI& uri, const QString& newName, bool isDirectory) {
		this->OldUri = uri;
		// 远端目录没法同步判断，只能由调用方告诉对话框；本机仍以实际类型为准。
		dirMode = isDirectory || OldUri.isDir();
		this->SubtitleLabel->setText(VITRL("YSS::editor.renameDialog.rename").arg(OldUri.toString()));
		this->Edit->setText(newName.isEmpty() ? OldUri.getFileName() : newName);
		onNewNameChanged(this->Edit->text());
	}
	YSSCore::General::YSSURI RenameDialog::targetUri(const QString& newName) const {
		const YSSCore::General::YSSURI parent = OldUri.parent();
		if (not parent.isValid()) {
			return YSSCore::General::YSSURI();
		}
		return parent.resolve(newName);
	}
	void RenameDialog::onNewNameChanged(const QString& newName) {
		validNewName = false;
		setConfirmEnabled(false);
		if (newName.isEmpty()) {
			setWarning(VITRL("YSS::editor.renameDialog.warning.empty"), true);
			return;
		}
		// 本机沿用 Windows 的非法字符集；远端按 POSIX 只把分隔符当作非法。
		const bool invalidChars = OldUri.isLocal()
			? newName.contains(QRegularExpression(R"([\/:*?"<>|])"))
			: newName.contains(QLatin1Char('/'));
		if (invalidChars or newName == "." or newName == "..") {
			setWarning(OldUri.isLocal()
				? VITRL("YSS::editor.renameDialog.warning.invalid")
				: VITRL("YSS::editor.renameDialog.warning.invalidPosix"), true);
			return;
		}
		const YSSCore::General::YSSURI newUri = targetUri(newName);
		if (not newUri.isValid()) {
			setWarning(VITRL("YSS::editor.renameDialog.warning.invalid"), true);
			return;
		}
		if (OldUri == newUri) {
			setWarning(VITRL("YSS::editor.renameDialog.warning.same"), true);
			return;
		}
		// 远端没法同步探测重名：交给重命名请求的 AlreadyExists 结果兜底。
		if (OldUri.isLocal() and QFileInfo(newUri.getLocalPath()).exists()) {
			setWarning(VITRL("YSS::editor.renameDialog.warning.exists"), true);
			return;
		}
		setWarning("", false);
		if (not dirMode and OldUri.getFileExt() != newUri.getFileExt() and not newUri.getFileExt().isEmpty()) {
			setWarning(VITRL("YSS::editor.renameDialog.warning.extNotSame.normal"), false);
			// notice: just warning, still valid
		}
		validNewName = true;
		setConfirmEnabled(true);
	}

	NewFolderDialog::NewFolderDialog() :QFrame() {
		this->setWindowFlags(Qt::Dialog);
		this->setFixedWidth(420);
		this->setWindowTitle(VITRL("YSS::editor.newFolderDialog.title"));

		MainLayout = new QVBoxLayout(this);
		this->setLayout(MainLayout);

		DescriptionLabel = new QLabel(this);
		DescriptionLabel->setWordWrap(true);
		MainLayout->addWidget(DescriptionLabel);

		NameEdit = new QLineEdit(this);
		MainLayout->addWidget(NameEdit);

		CheckLabel = new QLabel(this);
		CheckLabel->setWordWrap(true);
		MainLayout->addWidget(CheckLabel);

		QHBoxLayout* buttonLayout = new QHBoxLayout();
		ConfirmButton = new QPushButton(VITRL("Visindigo::general.confirm"), this);
		CancelButton = new QPushButton(VITRL("Visindigo::general.cancel"), this);
		buttonLayout->addStretch(1);
		buttonLayout->addWidget(ConfirmButton);
		buttonLayout->addWidget(CancelButton);
		MainLayout->addLayout(buttonLayout);

		connect(ConfirmButton, &QPushButton::clicked, this, [this]() {
			onConfirmClicked();
			});
		connect(CancelButton, &QPushButton::clicked, this, [this]() {
			this->close();
			});
		connect(NameEdit, &QLineEdit::textChanged, this, [this](const QString&) {
			validateName();
			});
	}

	void NewFolderDialog::setContext(const YSSCore::General::YSSURI& parentUri, const QStringList& existingNames) {
		this->ParentUri = parentUri;
		this->ExistingNames = existingNames;
		this->DescriptionLabel->setText(VITRL("YSS::editor.newFolderDialog.desc").arg(parentUri.toString()));
		this->NameEdit->clear();
		this->validateName();
	}

	void NewFolderDialog::validateName() {
		ValidName = false;
		QString newName = NameEdit->text().trimmed();
		if (newName.isEmpty()) {
			CheckLabel->setText(VITRL("YSS::editor.newFolderDialog.warning.empty"));
			CheckLabel->setStyleSheet("QLabel{color: red;}");
			return;
		}
		// 已列出的子项就是全部已知重名；远端没法同步探测，重名交给 mkdir 的 AlreadyExists 兜底。
		const bool local = ParentUri.isLocal();
		const bool invalidChars = local
			? newName.contains(QRegularExpression(R"([\/:*?"<>|])"))
			: newName.contains(QLatin1Char('/'));
		if (invalidChars or newName == "." or newName == "..") {
			CheckLabel->setText(VITRL("YSS::editor.newFolderDialog.warning.invalid"));
			CheckLabel->setStyleSheet("QLabel{color: red;}");
			return;
		}
		if (ExistingNames.contains(newName, local ? Qt::CaseInsensitive : Qt::CaseSensitive)) {
			CheckLabel->setText(VITRL("YSS::editor.newFolderDialog.warning.exists"));
			CheckLabel->setStyleSheet("QLabel{color: red;}");
			return;
		}
		CheckLabel->setText(QString());
		ValidName = true;
	}

	void NewFolderDialog::onConfirmClicked() {
		validateName();
		if (not ValidName) {
			return;
		}
		emit confirmed(NameEdit->text().trimmed());
		this->close();
	}
}