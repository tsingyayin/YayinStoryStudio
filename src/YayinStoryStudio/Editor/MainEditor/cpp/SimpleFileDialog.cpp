#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qregularexpression.h>
#include <QtWidgets/qmessagebox.h>
#include <General/TranslationHost.h>
#include "Editor/MainEditor/SimpleFileDialog.h"
namespace YSS::Editor {
	RenameDialog::RenameDialog() :QFrame() {
		this->setWindowFlags(Qt::Dialog);
		this->setFixedWidth(600);
		this->setWindowTitle(VITRL("YSS::editor.renameDialog.title"));
		TitleLabel = new QLabel(this);
		TitleLabel->setWordWrap(true);
		Edit = new QLineEdit(this);
		WarningLabel = new QLabel(this);
		ConfirmButton = new QPushButton(VITRL("Visindigo::general.confirm"), this);
		CancelButton = new QPushButton(VITRL("Visindigo::general.cancel"), this);
		Layout = new QVBoxLayout(this);
		this->setLayout(Layout);
		Layout->addWidget(TitleLabel);
		Layout->addWidget(Edit);
		Layout->addWidget(WarningLabel);
		QHBoxLayout* buttonLayout = new QHBoxLayout();
		buttonLayout->addWidget(ConfirmButton);
		buttonLayout->addWidget(CancelButton);
		Layout->addLayout(buttonLayout);
		connect(ConfirmButton, &QPushButton::clicked, this, [this]() {
			if (not validNewName) {
				return;
			}
			if (not dirMode) {
				QFileInfo fileInfo(AbsOldPath);
				QString newName = Edit->text();
				if (fileInfo.suffix() != QFileInfo(newName).suffix() && !QFileInfo(newName).suffix().isEmpty()) {
					bool ok = QMessageBox::question(this, VITRL("YSS::editor.renameDialog.warning.extNotSame.title"),
						VITRL("YSS::editor.renameDialog.warning.extNotSame.message").arg(QFileInfo(newName).suffix()).arg(fileInfo.suffix()),
						QMessageBox::Yes | QMessageBox::No,
						QMessageBox::No) == QMessageBox::Yes;
					if (not ok) {
						return;
					}
				}
				if (QFileInfo(newName).suffix().isEmpty()) {
					newName += "." + fileInfo.suffix();
				}
				emit renameConfirmed(AbsOldPath, fileInfo.absoluteDir().filePath(newName));
			}
			else {
				QDir dir(AbsOldPath);
				QString newName = Edit->text();
				QString newAbsPath = dir.absoluteFilePath(QString("..") + QDir::separator() + newName);
				newAbsPath = QDir(newAbsPath).absolutePath();
				emit renameConfirmed(AbsOldPath, newAbsPath);
			}

			this->close();
			});

		connect(CancelButton, &QPushButton::clicked, this, [this]() {
			this->close();
			});

		connect(Edit, &QLineEdit::textChanged, this, &RenameDialog::onNewNameChanged);
	}
	void RenameDialog::setContext(const QString& absOldPath, const QString& newName) {
		this->AbsOldPath = absOldPath;
		if (QFileInfo(absOldPath).isDir()) {
			dirMode = true;
		}
		this->TitleLabel->setText(VITRL("YSS::editor.renameDialog.rename").arg(absOldPath) + "\n\n" + VITRL("YSS::editor.renameDialog.request"));
		if (not dirMode) {
			if (newName.isEmpty()) {
				QFileInfo fileInfo(absOldPath);
				this->Edit->setText(fileInfo.fileName());
			}
			else {
				this->Edit->setText(newName);
			}
		}
		else {
			QDir dir(absOldPath);
			QString dirName = dir.dirName();
			if (newName.isEmpty()) {
				this->Edit->setText(dirName);
			}
			else {
				this->Edit->setText(newName);
			}
		}
		onNewNameChanged(this->Edit->text());
	}
	void RenameDialog::onNewNameChanged(const QString& newName) {
		validNewName = false;
		if (not dirMode) {
			QFileInfo fileInfo(AbsOldPath);
			QString newAbsPath = fileInfo.absoluteDir().filePath(newName);
			if (newName.isEmpty()) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.empty"));
				return;
			}
			QRegularExpression invalidCharsPattern(R"([\/:*?"<>|])");
			if (newName.contains(invalidCharsPattern)) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.invalid"));
				return;
			}
			QFileInfo newFileInfo(newAbsPath);
			if (AbsOldPath == newAbsPath) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.same"));
				return;
			}
			if (newFileInfo.exists()) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.exists"));
				return;
			}
			this->WarningLabel->setText("");
			if (fileInfo.suffix() != newFileInfo.suffix() && !newFileInfo.suffix().isEmpty()) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.extNotSame.normal"));
				// notice: just warning, still valid
			}
		}
		else {
			QDir dir(AbsOldPath);
			// 回退一级然后加上newName
			QString newAbsPath = dir.absoluteFilePath(QString("..") + QDir::separator() + newName);
			newAbsPath = QDir(newAbsPath).absolutePath();
			if (newName.isEmpty()) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.empty"));
				return;
			}
			QRegularExpression invalidCharsPattern(R"([\/:*?"<>|])");
			if (newName.contains(invalidCharsPattern)) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.invalid"));
				return;
			}
			QDir newDir(newAbsPath);
			if (AbsOldPath == newAbsPath) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.same"));
				return;
			}
			if (newDir.exists()) {
				this->WarningLabel->setText(VITRL("YSS::editor.renameDialog.warning.exists"));
				return;
			}
			this->WarningLabel->setText("");
		}
		validNewName = true;
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

	void NewFolderDialog::setContext(const QString& parentPath, const QStringList& existingNames) {
		this->ParentPath = parentPath;
		this->ExistingNames = existingNames;
		this->DescriptionLabel->setText(VITRL("YSS::editor.newFolderDialog.desc").arg(parentPath));
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
		QRegularExpression invalidCharsPattern(R"([\/:*?"<>|])");
		if (newName.contains(invalidCharsPattern)) {
			CheckLabel->setText(VITRL("YSS::editor.newFolderDialog.warning.invalid"));
			CheckLabel->setStyleSheet("QLabel{color: red;}");
			return;
		}
		if (ExistingNames.contains(newName) || QFileInfo(ParentPath + QDir::separator() + newName).exists()) {
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