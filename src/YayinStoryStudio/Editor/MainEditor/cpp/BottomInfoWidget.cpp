#include "Editor/MainEditor/BottomInfoWidget.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qprogressbar.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/VIApplication.h>
#include <Widgets/ThemeManager.h>
#include <General/TranslationHost.h>
namespace YSS::Editor {
	class BottomInfoWidgetPrivate {
		friend class BottomInfoWidget;
	protected:
		QHBoxLayout* MainLayout;
		QToolButton* ProcessingMessagesButton;

		QHBoxLayout* DebugInfoLayout;
		YSSCore::Editor::DebugServer::DebugAction DebugAction = YSSCore::Editor::DebugServer::DebugAction::Unknown;

		QWidget* DebugInfoWidget;
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

		void initFontSizeComboBox() {
			EditorFontSizeComboBox->addItem("50%", 0.5f);
			EditorFontSizeComboBox->addItem("75%", 0.75f);
			EditorFontSizeComboBox->addItem("100%", 1.0f);
			EditorFontSizeComboBox->addItem("125%", 1.25f);
			EditorFontSizeComboBox->addItem("150%", 1.5f);
			EditorFontSizeComboBox->addItem("175%", 1.75f);
			EditorFontSizeComboBox->addItem("200%", 2.0f);
			EditorFontSizeComboBox->setCurrentIndex(2);
		}
	};

	BottomInfoWidget::BottomInfoWidget(QWidget* parent) :BorderFrame(parent), d(new BottomInfoWidgetPrivate) {
		d->MainLayout = new QHBoxLayout(this);
		d->MainLayout->setContentsMargins(0, 0, 0, 0);
		d->MainLayout->setSpacing(30);
		d->ProcessingMessagesButton = new QToolButton(this);
		d->ProcessingMessagesButton->setIconSize(QSize(d->iconSize, d->iconSize));
		d->MainLayout->addWidget(d->ProcessingMessagesButton);

		d->DebugInfoWidget = new QWidget(this);
		d->DebugInfoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		d->DebugInfoLayout = new QHBoxLayout(d->DebugInfoWidget);
		d->DebugInfoLayout->setContentsMargins(0, 0, 0, 0);
		d->DebugInfoLayout->setSpacing(5);
		d->DebugInfoIcon = new QLabel(d->DebugInfoWidget);
		d->DebugInfoText = new QLabel(d->DebugInfoWidget);
		d->DebugProgressBar = new QProgressBar(d->DebugInfoWidget);
		d->DebugProgressBar->setTextVisible(false);
		d->DebugProgressBar->setMinimum(0);
		d->DebugProgressBar->setMaximum(100);
		d->DebugProgressBar->setValue(20);
		d->DebugProgressBar->setVisible(true);
		d->DebugProgressBar->setFixedWidth(100);
		d->DebugInfoLayout->addWidget(d->DebugInfoIcon);
		d->DebugInfoLayout->addWidget(d->DebugInfoText);
		d->DebugInfoLayout->addWidget(d->DebugProgressBar);
		d->MainLayout->addWidget(d->DebugInfoWidget);

		d->EditorInfoWidget = new QWidget(this);
		d->EditorInfoWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
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
		d->EditorInfoText->setText("");
		d->EditorFontSizeComboBox = new QComboBox(d->EditorInfoWidget);
		d->initFontSizeComboBox();
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

		displayDebugProgress(YSSCore::Editor::DebugServer::DebugAction::Unknown, 0, 0);
		clearDebugProgress();
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
	}
	
	void BottomInfoWidget::displayDebugInfo(YSSCore::Editor::DebugServer::DebugAction action, const QString& info) {
		d->DebugAction = action;
		d->applyIcon();
		d->DebugInfoText->setText(info);
	}

	void BottomInfoWidget::clearDebugInfo() {
		d->DebugAction = YSSCore::Editor::DebugServer::DebugAction::Unknown;
		d->applyIcon();
		d->DebugInfoText->setText("");
	}

	void BottomInfoWidget::displayDebugProgress(YSSCore::Editor::DebugServer::DebugAction action, qint32 finished, qint32 total) {
		d->DebugAction = action;
		d->applyIcon();
		if (total > 0) {
			if (d->DebugProgressBar->maximum() != 100) {
				d->DebugProgressBar->setMaximum(100);
			}
			d->DebugProgressBar->setValue(finished * 100 / total);
			d->DebugProgressBar->setVisible(true);
		}
		else if (total == 0 && finished == 0) {
			d->DebugProgressBar->setMaximum(0); // busy indicator
			d->DebugProgressBar->setVisible(true);
		}
	}

	void BottomInfoWidget::clearDebugProgress() {
		d->DebugProgressBar->setVisible(false);
	}

	void BottomInfoWidget::setEditorFontSize(float fontSize) {
		// todo.
		// need to handle freedom input logic. Will not implement immediately.
	}

	void BottomInfoWidget::displayEditorInfo(qint32 totalLine, qint32 currentLine, qint32 currentColumn, qint32 selected) {
		QString totalLines = VITR("YSS::editor.textEdit.totalLines").arg(totalLine) + ". ";
		if (selected > 0) {
			totalLines += VITR("YSS::editor.textEdit.cursorInfo_s").arg(currentLine).arg(currentColumn).arg(selected);
		}else{
			totalLines += VITR("YSS::editor.textEdit.cursorInfo").arg(currentLine).arg(currentColumn);
		}
		d->EditorInfoText->setText(totalLines);
	}

	void BottomInfoWidget::displayEditorInfo(const QTextCursor& cursor) {
		qint32 totalLine = cursor.document()->blockCount();
		qint32 currentLine = cursor.blockNumber() + 1;
		qint32 currentColumn = cursor.positionInBlock();
		qint32 selected = cursor.selectedText().length();
		displayEditorInfo(totalLine, currentLine, currentColumn, selected);
	}

	void BottomInfoWidget::setEditorInfoEnable(bool enable) {
		d->EditorInfoEnable = enable;
		if (not enable) {
			d->EditorInfoWidget->hide();
		}
		else {
			d->EditorInfoWidget->show();
		}
	}

	void BottomInfoWidget::displayFileMessageCount(qint32 error, qint32 warning, qint32 info) {
		d->FM_ErrorText->setText(QString::number(error));
		d->FM_WarningText->setText(QString::number(warning));
		d->FM_InfoText->setText(QString::number(info));
	}

	void BottomInfoWidget::displayFileMessageCount(const std::tuple<qint32, qint32, qint32>& values) {
		d->FM_ErrorText->setText(QString::number(std::get<0>(values)));
		d->FM_WarningText->setText(QString::number(std::get<1>(values)));
		d->FM_InfoText->setText(QString::number(std::get<2>(values)));
	}
	void BottomInfoWidget::displayGitInfo(qint32 pull, qint32 push, qint32 modified, QStringList branchs, QString currentBranch) {
		d->GI_PushPullText->setText(QString("%1 / %2").arg(pull).arg(push));
		d->GI_ModifiedText->setText(QString::number(modified));
		d->GI_BranchText->setText(currentBranch);
	}

	void BottomInfoWidget::setGitInfoEnable(bool enable) {
		d->GitInfoEnable = enable;
		if (not enable) {
			d->GitInfoWidget->hide();
		}
		else {
			d->GitInfoWidget->show();
		}
	}

}