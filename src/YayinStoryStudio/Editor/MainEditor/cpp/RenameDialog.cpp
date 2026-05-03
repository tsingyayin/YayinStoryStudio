#include "Editor/MainEditor/RenameDialog.h"
#include <QtCore/qfileinfo.h>
#include <General/TranslationHost.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qdir.h>
namespace YSS::Editor {
	RenameDialog::RenameDialog() :QFrame() {
		this->setWindowFlags(Qt::Dialog);
		this->setFixedWidth(400);
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
			QFileInfo fileInfo(AbsOldPath);
			QString newName = Edit->text();
			emit renameConfirmed(AbsOldPath, fileInfo.absoluteDir().filePath(newName));
			this->close();
			});
		
		connect(CancelButton, &QPushButton::clicked, this, [this]() {
			this->close();
			});
		
		connect(Edit, &QLineEdit::textChanged, this, &RenameDialog::onNewNameChanged);
	
	}
	void RenameDialog::setContext(const QString& absOldPath, const QString& newName) {
		this->AbsOldPath = absOldPath;
		this->TitleLabel->setText(VITR("YSS::editor.renameDialog.rename").arg(absOldPath));
		this->Edit->setText(newName);
		this->WarningLabel->setText("");
	}
	void RenameDialog::onNewNameChanged(const QString& newName) {
		validNewName = false;
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
		validNewName = true;
	}
}