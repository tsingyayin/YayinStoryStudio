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
		QToolButton* EditorInfoText;
		QComboBox* EditorFontSizeComboBox;
		QComboBox* EditorTabCompleterLevelComboBox;

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
				FM_ErrorIcon->setPixmap(VIApp->getFontIcon("\uEA39", iconSize, { QColor("#EB3324")}).pixmap(iconSize, iconSize));
				FM_WarningIcon->setPixmap(VIApp->getFontIcon("\uE7BA", iconSize, { QColor("#F5F352")}).pixmap(iconSize, iconSize));
				FM_InfoIcon->setPixmap(VIApp->getFontIcon("\uE946", iconSize, { QColor("#3282F6")}).pixmap(iconSize, iconSize));
				GI_PushPullIcon->setPixmap(VIApp->getFontIcon("\uE8CB", iconSize, { textColor }).pixmap(iconSize, iconSize));
				GI_ModifiedIcon->setPixmap(VIApp->getFontIcon("\uE70F", iconSize, { textColor }).pixmap(iconSize, iconSize));
				GI_BranchIcon->setPixmap(VIApp->getFontIcon("\uF003", iconSize, { textColor }).pixmap(iconSize, iconSize));
				ProgramMessagesButton->setIcon(VIApp->getFontIcon("\uEA8F", iconSize, { textColor }).pixmap(iconSize, iconSize));
			}
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
				DebugInfoIcon->setPixmap(VIApp->getFontIcon("\uE8BD", iconSize, { textColor }).pixmap(iconSize, iconSize));
				break;
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
			qint32 index = -1;
			float configFontSize = YSS::Editor::TextEditConfigOperator::getFontScale();
			for (qint32 i = 0; i < EditorFontSizeComboBox->count(); i++) {
				if (EditorFontSizeComboBox->itemData(i).toFloat() == configFontSize) {
					index = i;
					break;
				}
			}
			if (index == -1) {
				index = 2; // Default to 100% if not found
			}
			EditorFontSizeComboBox->setCurrentIndex(index);
		}

		void initTabCompleterLevelComboBox() {
			EditorTabCompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.none"), YSSCore::Editor::TabCompleterItem::CompleterLevel::None);
			EditorTabCompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.few"), YSSCore::Editor::TabCompleterItem::CompleterLevel::Few);
			EditorTabCompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.some"), YSSCore::Editor::TabCompleterItem::CompleterLevel::Some);
			EditorTabCompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.many"), YSSCore::Editor::TabCompleterItem::CompleterLevel::Many);
			EditorTabCompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.all"), YSSCore::Editor::TabCompleterItem::CompleterLevel::All);
			qint32 index = -1;
			auto configLevel = YSS::Editor::TextEditConfigOperator::getCompleterLevel();
			for (qint32 i = 0; i < EditorTabCompleterLevelComboBox->count(); i++) {
				if (EditorTabCompleterLevelComboBox->itemData(i).value<YSSCore::Editor::TabCompleterItem::CompleterLevel>() == configLevel) {
					index = i;
					break;
				}
			}
			if (index == -1) {
				index = 4; // Default to "All" if not found
			}
			EditorTabCompleterLevelComboBox->setCurrentIndex(index);
		}
	};

	BottomInfoWidget::BottomInfoWidget(QWidget* parent) :QFrame(parent), d(new BottomInfoWidgetPrivate) {
		d->MainLayout = new QHBoxLayout(this);
		d->MainLayout->setContentsMargins(0, 0, 0, 0);
		d->MainLayout->setSpacing(30);
		d->ProcessingMessagesButton = new QToolButton(this);
		d->ProcessingMessagesButton->setIconSize(QSize(d->iconSize, d->iconSize));
		d->ProcessingMessagesButton->hide(); // not implemented in 0.16.
		d->MainLayout->addWidget(d->ProcessingMessagesButton);

		d->DebugInfoWidget = new QWidget(this);
		d->DebugInfoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		d->DebugInfoLayout = new QHBoxLayout(d->DebugInfoWidget);
		d->DebugInfoLayout->setContentsMargins(0, 0, 0, 0);
		d->DebugInfoLayout->setSpacing(5);
		d->DebugInfoIcon = new QLabel(d->DebugInfoWidget);
		d->DebugInfoIcon->setFixedSize(d->iconSize, d->iconSize);
		d->DebugInfoText = new QLabel(d->DebugInfoWidget);
		d->DebugInfoText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		d->DebugInfoText->setText(VITRL("YSS::editor.bottomInfoWidget.debugMessage.prepared"));
		d->DebugProgressBar = new QProgressBar(d->DebugInfoWidget);
		d->DebugProgressBar->setTextVisible(false);
		d->DebugProgressBar->setMinimum(0);
		d->DebugProgressBar->setMaximum(100);
		d->DebugProgressBar->setValue(20);
		d->DebugProgressBar->setVisible(true);
		d->DebugProgressBar->setFixedWidth(100);
		d->DebugInfoLayout->addWidget(d->DebugInfoIcon);
		d->DebugInfoLayout->addWidget(d->DebugProgressBar);
		d->DebugInfoLayout->addWidget(d->DebugInfoText);
		d->MainLayout->addWidget(d->DebugInfoWidget);

		d->EditorInfoWidget = new QWidget(this);
		d->EditorInfoWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		d->EditorInfoLayout = new QHBoxLayout(d->EditorInfoWidget);
		d->EditorInfoLayout->setContentsMargins(10, 0, 10, 0);
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
		d->EditorInfoText = new QToolButton(d->EditorInfoWidget);
		d->EditorInfoText->setAutoRaise(true); // borderless, only shows frame on hover
		d->EditorInfoText->setText("");
		d->EditorInfoText->setToolTip(VITRL("YSS::editor.bottomInfoWidget.statistic.tooltip"));
		connect(d->EditorInfoText, &QToolButton::clicked, this, [this]() {
			emit requestStatistic();
			});
		d->EditorFontSizeComboBox = new QComboBox(d->EditorInfoWidget);
		d->EditorFontSizeComboBox->setStyleSheet("QComboBox { border: none; background: transparent; }");
		d->EditorTabCompleterLevelComboBox = new QComboBox(d->EditorInfoWidget);
		d->EditorTabCompleterLevelComboBox->setStyleSheet("QComboBox { border: none; background: transparent; }");
		d->initFontSizeComboBox();
		d->initTabCompleterLevelComboBox();
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
		d->EditorInfoLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Minimum));
		d->EditorInfoLayout->addWidget(d->EditorFontSizeComboBox);
		d->EditorInfoLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Minimum));
		d->EditorInfoLayout->addWidget(d->EditorTabCompleterLevelComboBox);
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
		d->ProgramMessagesButton->hide(); // not implemented in 0.16.
		d->MainLayout->addWidget(d->ProgramMessagesButton);

		displayDebugProgress(YSSCore::Editor::DebugServer::DebugAction::Unknown, 0, 0);
		clearDebugProgress();
		setColorfulEnable(true);
		onThemeChanged();

		connect(d->EditorFontSizeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
			if (index >= 0) {
				float fontSize = d->EditorFontSizeComboBox->itemData(index).toFloat();
				setEditorFontSize(fontSize);
			}
			});

		connect(d->EditorTabCompleterLevelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
			if (index >= 0) {
				YSSCore::Editor::TabCompleterItem::CompleterLevel level = 
					static_cast<YSSCore::Editor::TabCompleterItem::CompleterLevel>(d->EditorTabCompleterLevelComboBox->itemData(index).toInt());
				YSS::Editor::TextEditConfigOperator::setCompleterLevel(level, true);
			}
			});
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
		d->DebugInfoText->setText(VITRL("YSS::editor.bottomInfoWidget.debugMessage.prepared"));
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
		else if (total == -1 && finished == -1) {
			d->DebugProgressBar->setMaximum(100);
			d->DebugProgressBar->setValue(0);
			d->DebugProgressBar->setVisible(false);
		}
	}

	void BottomInfoWidget::clearDebugProgress() {
		d->DebugProgressBar->setVisible(false);
	}

	void BottomInfoWidget::setEditorFontSize(float fontSize) {
		if (fontSize <= 0.2f) fontSize = 0.2f;
		if (fontSize >= 5.0f) fontSize = 5.0f;
		YSS::Editor::TextEditConfigOperator::setFontScale(fontSize, true);
	}

	void BottomInfoWidget::displayEditorInfo(qint32 totalLine, qint32 currentLine, qint32 currentColumn, qint32 selected) {
		QString totalLines = VITRL("YSS::editor.bottomInfoWidget.totalLines").arg(totalLine) + ". ";
		if (selected > 0) {
			totalLines += VITRL("YSS::editor.bottomInfoWidget.cursorInfo_s").arg(currentLine).arg(currentColumn).arg(selected);
		}else{
			totalLines += VITRL("YSS::editor.bottomInfoWidget.cursorInfo").arg(currentLine).arg(currentColumn);
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