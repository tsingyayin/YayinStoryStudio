#include "Editor/MainEditor/RenameDialog.h"
#include <QtCore/qfileinfo.h>
#include <General/TranslationHost.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qdir.h>
#include <QtWidgets/qmessagebox.h>
namespace YSS::Editor {
	RenameDialog::RenameDialog() :QFrame() {
		this->setWindowFlags(Qt::Dialog);
		this->setFixedWidth(600);
		this->setWindowTitle(VITR("YSS::editor.renameDialog.title"));
		TitleLabel = new QLabel(this);
		TitleLabel->setWordWrap(true);
		Edit = new QLineEdit(this);
		WarningLabel = new QLabel(this);
		ConfirmButton = new QPushButton(VITR("Visindigo::general.confirm"), this);
		CancelButton = new QPushButton(VITR("Visindigo::general.cancel"), this);
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
					bool ok = QMessageBox::question(this, VITR("YSS::editor.renameDialog.warning.extNotSame.title"),
						VITR("YSS::editor.renameDialog.warning.extNotSame.message").arg(QFileInfo(newName).suffix()).arg(fileInfo.suffix()),
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
		this->TitleLabel->setText(VITR("YSS::editor.renameDialog.rename").arg(absOldPath) + "\n\n" + VITR("YSS::editor.renameDialog.request"));
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
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.empty"));
				return;
			}
			QRegularExpression invalidCharsPattern(R"([\/:*?"<>|])");
			if (newName.contains(invalidCharsPattern)) {
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.invalid"));
				return;
			}
			QFileInfo newFileInfo(newAbsPath);
			if (AbsOldPath == newAbsPath) {
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.same"));
				return;
			}
			if (newFileInfo.exists()) {
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.exists"));
				return;
			}
			this->WarningLabel->setText("");
			if (fileInfo.suffix() != newFileInfo.suffix() && !newFileInfo.suffix().isEmpty()) {
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.extNotSame.normal"));
				// notice: just warning, still valid
			}
		}
		else {
			QDir dir(AbsOldPath);
			// 回退一级然后加上newName
			QString newAbsPath = dir.absoluteFilePath(QString("..") + QDir::separator() + newName);
			newAbsPath = QDir(newAbsPath).absolutePath();
			if (newName.isEmpty()) {
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.empty"));
				return;
			}
			QRegularExpression invalidCharsPattern(R"([\/:*?"<>|])");
			if (newName.contains(invalidCharsPattern)) {
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.invalid"));
				return;
			}
			QDir newDir(newAbsPath);
			if (AbsOldPath == newAbsPath) {
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.same"));
				return;
			}
			if (newDir.exists()) {
				this->WarningLabel->setText(VITR("YSS::editor.renameDialog.warning.exists"));
				return;
			}
			this->WarningLabel->setText("");
		}
		validNewName = true;
	}
}