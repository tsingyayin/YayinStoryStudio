#include "Editor/MainEditor/BottomInfoWidget.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qprogressbar.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/VIApplication.h>

namespace YSS::Editor {
	class BottomInfoWidgetPrivate {
		friend class BottomInfoWidget;
	protected:
		QHBoxLayout* MainLayout;
		QToolButton* ProcessingMessagesButton;

		QHBoxLayout* DebugInfoLayout;
		YSSCore::Editor::DebugServer::DebugAction DebugAction = YSSCore::Editor::DebugServer::DebugAction::Unknown;
		QLabel* DebugInfoIcon;
		QLabel* DebugInfoText;
		QProgressBar* DebugProgressBar;
		
		QWidget* EditorInfoWidget;
		QHBoxLayout* EditorInfoLayout;
		QLabel* FM_ErrorIcon;
		QLabel* FM_ErrorText;
		QLabel* FM_WarningIcon;
		QLabel* FM_WarningText;
		QLabel* FM_InfoIcon;
		QLabel* FM_InfoText;
		QLabel* EditorInfoText;
		QComboBox* EditorFontSizeComboBox;

		QWidget* GitInfoWidget;
		QHBoxLayout* GitInfoLayout;
		QLabel* GI_PushPullIcon;
		QLabel* GI_PushPullText;
		QLabel* GI_ModifiedIcon;
		QLabel* GI_ModifiedText;
		QLabel* GI_BranchIcon;
		QLabel* GI_BranchText;

		QToolButton* ProgramMessagesButton;

		QColor textColor;
		qint32 iconSize = 30;
		bool EditorInfoEnable = true;
		bool GitInfoEnable = true;
		void applyIcon() {
			if (textColor != VISTM->getPaletteTextColor()) {
				textColor = VISTM->getPaletteTextColor();
				ProcessingMessagesButton->setIcon(VIApp->getFontIcon("\uE8BD", iconSize, { textColor }));
				switch (DebugAction)
				{
				case YSSCore::Editor::DebugServer::DebugAction::DebugRun:
					DebugInfoIcon->setPixmap(VIApp->getFontIcon("\uE9D9", iconSize, { textColor }).pixmap(iconSize, iconSize));
					break;
				case YSSCore::Editor::DebugServer::DebugAction::Run:
					DebugInfoIcon->setPixmap(VIApp->getFontIcon("\uE768", iconSize, { textColor }).pixmap(iconSize, iconSize));
					break;
				case YSSCore::Editor::DebugServer::DebugAction::Build:
					DebugInfoIcon->setPixmap(VIApp->getFontIcon("\uEC7A", iconSize, { textColor }).pixmap(iconSize, iconSize));
					break;
				default:
					DebugInfoIcon->setPixmap(QPixmap());
					break;
				}
				FM_ErrorIcon->setPixmap(VIApp->getFontIcon("\uEA39", iconSize, { textColor }).pixmap(iconSize, iconSize));
				FM_WarningIcon->setPixmap(VIApp->getFontIcon("\uE7BA", iconSize, { textColor }).pixmap(iconSize, iconSize));
				FM_InfoIcon->setPixmap(VIApp->getFontIcon("\uE946", iconSize, { textColor }).pixmap(iconSize, iconSize));
				GI_PushPullIcon->setPixmap(VIApp->getFontIcon("\uE8CB", iconSize, { textColor }).pixmap(iconSize, iconSize));
				GI_ModifiedIcon->setPixmap(VIApp->getFontIcon("\uE70F", iconSize, { textColor }).pixmap(iconSize, iconSize));
				GI_BranchIcon->setPixmap(VIApp->getFontIcon("\uF003", iconSize, { textColor }).pixmap(iconSize, iconSize));
				ProgramMessagesButton->setIcon(VIApp->getFontIcon("\uEA8F", iconSize, { textColor }).pixmap(iconSize, iconSize));
			}
		}
	};

	BottomInfoWidget::BottomInfoWidget(QWidget* parent) :BorderFrame(parent), d(new BottomInfoWidgetPrivate) {
		d->MainLayout = new QHBoxLayout(this);
		d->MainLayout->setContentsMargins(0, 0, 0, 0);
		d->MainLayout->setSpacing(30);
		d->ProcessingMessagesButton = new QToolButton(this);
		d->ProcessingMessagesButton->setIconSize(QSize(d->iconSize, d->iconSize));
		d->MainLayout->addWidget(d->ProcessingMessagesButton);

		d->DebugInfoLayout = new QHBoxLayout();
		d->DebugInfoLayout->setContentsMargins(0, 0, 0, 0);
		d->DebugInfoLayout->setSpacing(5);
		d->DebugInfoIcon = new QLabel(this);
		d->DebugInfoText = new QLabel(this);
		d->DebugProgressBar = new QProgressBar(this);
		d->DebugProgressBar->setTextVisible(false);
		d->DebugProgressBar->setMinimum(0);
		d->DebugProgressBar->setMaximum(100);
		d->DebugProgressBar->setValue(20);
		d->DebugProgressBar->setVisible(true);
		d->DebugProgressBar->setFixedWidth(100);
		d->DebugInfoLayout->addWidget(d->DebugInfoIcon);
		d->DebugInfoLayout->addWidget(d->DebugInfoText);
		d->DebugInfoLayout->addWidget(d->DebugProgressBar);
		d->MainLayout->addLayout(d->DebugInfoLayout);

		d->EditorInfoWidget = new QWidget(this);
		d->EditorInfoLayout = new QHBoxLayout(d->EditorInfoWidget);
		d->EditorInfoLayout->setContentsMargins(0, 0, 0, 0);
		d->EditorInfoLayout->setSpacing(5);
		d->FM_ErrorIcon = new QLabel(d->EditorInfoWidget);
		d->FM_ErrorText = new QLabel(d->EditorInfoWidget);
		d->FM_ErrorText->setText("0");
		d->FM_WarningIcon = new QLabel(d->EditorInfoWidget);
		d->FM_WarningText = new QLabel(d->EditorInfoWidget);
		d->FM_WarningText->setText("0");
		d->FM_InfoIcon = new QLabel(d->EditorInfoWidget);
		d->FM_InfoText = new QLabel(d->EditorInfoWidget);
		d->FM_InfoText->setText("0");
		d->EditorInfoText = new QLabel(d->EditorInfoWidget);
		d->EditorInfoText->setText("行 1 （共 1）列 1（已选中1）");
		d->EditorFontSizeComboBox = new QComboBox(d->EditorInfoWidget);
		d->EditorFontSizeComboBox->addItem("100%", 100);
		d->EditorInfoLayout->addWidget(d->FM_ErrorIcon);
		d->EditorInfoLayout->addWidget(d->FM_ErrorText);
		d->EditorInfoLayout->addWidget(d->FM_WarningIcon);
		d->EditorInfoLayout->addWidget(d->FM_WarningText);
		d->EditorInfoLayout->addWidget(d->FM_InfoIcon);
		d->EditorInfoLayout->addWidget(d->FM_InfoText);
		QWidget* spacer = new QWidget(d->EditorInfoWidget);
		spacer->setFixedWidth(20);
		d->EditorInfoLayout->addWidget(spacer);
		d->EditorInfoLayout->addWidget(d->EditorInfoText);
		d->EditorInfoLayout->addWidget(d->EditorFontSizeComboBox);
		d->MainLayout->addWidget(d->EditorInfoWidget);

		d->GitInfoWidget = new QWidget(this);
		d->GitInfoLayout = new QHBoxLayout(d->GitInfoWidget);
		d->GitInfoLayout->setContentsMargins(0, 0, 0, 0);
		d->GitInfoLayout->setSpacing(5);
		d->GI_PushPullIcon = new QLabel(d->GitInfoWidget);
		d->GI_PushPullText = new QLabel(d->GitInfoWidget);
		d->GI_PushPullText->setText("0 / 0");
		d->GI_ModifiedIcon = new QLabel(d->GitInfoWidget);
		d->GI_ModifiedText = new QLabel(d->GitInfoWidget);
		d->GI_ModifiedText->setText("0");
		d->GI_BranchIcon = new QLabel(d->GitInfoWidget);
		d->GI_BranchText = new QLabel(d->GitInfoWidget);
		d->GI_BranchText->setText("(unknown)");
		d->GitInfoLayout->addWidget(d->GI_PushPullIcon);
		d->GitInfoLayout->addWidget(d->GI_PushPullText);
		d->GitInfoLayout->addWidget(d->GI_ModifiedIcon);
		d->GitInfoLayout->addWidget(d->GI_ModifiedText);
		d->GitInfoLayout->addWidget(d->GI_BranchIcon);
		d->GitInfoLayout->addWidget(d->GI_BranchText);
		d->MainLayout->addWidget(d->GitInfoWidget);

		d->ProgramMessagesButton = new QToolButton(this);
		d->ProgramMessagesButton->setIconSize(QSize(d->iconSize, d->iconSize));
		d->MainLayout->addWidget(d->ProgramMessagesButton);

		setColorfulEnable(true);
		onThemeChanged();
	}

	BottomInfoWidget::~BottomInfoWidget() {
		delete d;
	}

	void BottomInfoWidget::onThemeChanged() {
		d->applyIcon();
	}

	void BottomInfoWidget::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		d->ProcessingMessagesButton->setFixedSize(this->height(), this->height());
		d->ProgramMessagesButton->setFixedSize(this->height(), this->height());

		if (d->EditorInfoEnable) {
			if (this->width() < 1000) {
				d->EditorFontSizeComboBox->hide();
			}
			else {
				d->EditorFontSizeComboBox->show();
			}
			if (this->width() < 950) {
				d->FM_InfoIcon->hide();
				d->FM_InfoText->hide();
			}
			else {
				d->FM_InfoIcon->show();
				d->FM_InfoText->show();
			}
			if (this->width() < 900) {
				d->EditorInfoText->hide();
			}
			else {
				d->EditorInfoText->show();
			}
			
			if (this->width() < 850) {
				d->EditorInfoWidget->hide();
			}
			else {
				d->EditorInfoWidget->show();
			}
		}
		if (d->GitInfoEnable) {
			if (this->width() < 700) {
				d->GitInfoWidget->hide();
			}
			else {
				d->GitInfoWidget->show();
			}
		}
	}

}