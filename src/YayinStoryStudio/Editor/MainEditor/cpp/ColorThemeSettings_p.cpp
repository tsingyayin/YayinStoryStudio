#include "Editor/MainEditor/private/ColorThemeSettings_p.h"
#include <General/TranslationHost.h>
namespace YSS::Editor {
	ColorThemeSettingsCopyDialog::ColorThemeSettingsCopyDialog(QWidget* parent) :QWidget(parent) {
		this->setWindowFlags(Qt::Dialog);
		this->setFixedWidth(420);
		this->setWindowTitle(VITRL("YSS::colorThemeSettings.duplicateDialog.title"));

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

	ColorThemeSettingsCopyDialog::~ColorThemeSettingsCopyDialog() {}

	void ColorThemeSettingsCopyDialog::setCurrentThemes(const QStringList& themes) {
		CurrentThemes = themes;
	}

	void ColorThemeSettingsCopyDialog::setFromThemeName(const QString& name) {
		FromThemeName = name;
		DescriptionLabel->setText(VITRL("YSS::colorThemeSettings.duplicateDialog.desc").arg(name));
		validateName();
	}

	void ColorThemeSettingsCopyDialog::validateName() {
		ValidName = false;
		QString newName = NameEdit->text().trimmed();
		if (newName.isEmpty()) {
			CheckLabel->setText(VITRL("YSS::colorThemeSettings.duplicateDialog.warning.empty"));
			CheckLabel->setStyleSheet("QLabel{color: red;}");
			return;
		}
		if (newName == FromThemeName) {
			CheckLabel->setText(VITRL("YSS::colorThemeSettings.duplicateDialog.warning.same"));
			CheckLabel->setStyleSheet("QLabel{color: red;}");
			return;
		}
		if (CurrentThemes.contains(newName)) {
			CheckLabel->setText(VITRL("YSS::colorThemeSettings.duplicateDialog.warning.exists"));
			CheckLabel->setStyleSheet("QLabel{color: red;}");
			return;
		}
		CheckLabel->setText(QString());
		ValidName = true;
	}

	void ColorThemeSettingsCopyDialog::onConfirmClicked() {
		validateName();
		if (not ValidName) {
			return;
		}
		emit confirmed(NameEdit->text().trimmed());
		this->close();
	}
}
