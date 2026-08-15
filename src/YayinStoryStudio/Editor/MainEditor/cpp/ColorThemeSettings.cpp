#include "Editor/MainEditor/ColorThemeSettings.h"
#include "Editor/MainEditor/private/ColorThemeSettings_p.h"
#include "Editor/MainEditor/TextEditConfigOperator.h"
#include <General/TranslationHost.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qfontcombobox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qlabel.h>
#include <Editor/ColorThemeProvider.h>
#include <Editor/TextEdit.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qlistview.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtWidgets/qframe.h>
#include <Widgets/BorderFrame.h>
#include <Editor/LangServerManager.h>
#include <Editor/ColorThemeProvider.h>
#include <Editor/LangServer.h>
#include <QtWidgets/qmessagebox.h>
#include <Editor/SyntaxHighlighter.h>
#include <QtWidgets/qtextedit.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qfont.h>
#include <QtWidgets/qcolordialog.h>
#include <QtGui/qpalette.h>
#include <QtCore/qmetaobject.h>
namespace YSS::Editor {
	ColorThemeSettingsVFServer::ColorThemeSettingsVFServer(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("Color Theme Settings Virtual File Server", "YSS.Editor.ColorThemeSettingsVFServer", plugin) {
		setEditorType(EditorType::BuiltInEditor);
		setSupportedFileExts({ "YSS.MainEditor.ColorThemeSettings" });
		setAsVitrualFileServer(true);
	}

	YSSCore::Editor::FileEditWidget* ColorThemeSettingsVFServer::onCreateFileEditWidget() {
		return new ColorThemeSettingsEditWidget();
	}

	class ColorThemeSettingsEditWidgetPrivate {
		friend class ColorThemeSettingsEditWidget;
	protected:
		bool changed = false;
	
		QList<YSSCore::Editor::LangServer*> langServerList;
		YSSCore::Editor::LangServer* currentLangServer = nullptr;
		QMap<QString, YSSCore::Editor::StyleData> themeStyleDataBackup;
		QMap<QString, YSSCore::Editor::StyleData> themeStyleDataEditing;
		QString currentThemeName;
		bool themeStyleDataEditingChanged = false;
		bool updatingControls = false;
		QGridLayout* mainLayout;

		Visindigo::Widgets::BorderFrame* langServerFrame;
		QGridLayout* langServerLayout;
		QLabel* langServerTitleLabel;
		QComboBox* langServerComboBox;

		Visindigo::Widgets::BorderFrame* themeFrame;
		QGridLayout* themeLayout;
		QLabel* themeTitleLabel;
		QComboBox* themeComboBox;
		QPushButton* duplicateButton;
		QPushButton* deleteButton;

		Visindigo::Widgets::BorderFrame* styleFrame;
		QGridLayout* styleLayout;
		QLabel* fontTitleLabel;
		QFontComboBox* fontComboBox;

		YSSCore::Editor::TextEdit* documentPreviewTextEdit;
		QTextEdit* previewTextEdit;

		Visindigo::Widgets::BorderFrame* editFrame;
		QGridLayout* editLayout;
		QListView* configNodeView;
		QLabel* textColorTitleLabel;
		QPushButton* textColorButton;
		QLabel* bgColorTitleLabel;
		QPushButton* bgColorButton;
		QLabel* lineColorTitleLabel;
		QPushButton* lineColorButton;
		QLabel* lineTypeTitleLabel;
		QComboBox* lineTypeComboBox;
		QCheckBox* useBoldCheckBox;
		QCheckBox* useItalicCheckBox;
		QCheckBox* useBgColorCheckBox;

		QLabel* staticWarningLabel;
		QPushButton* saveButton;
		QPushButton* resetButton;

		QLabel* nothingToEditLabel;

		bool initLangServer() {
			QList<YSSCore::Editor::LangServer*> servers = YSSCore::Editor::LangServerManager::getInstance()->getAllServers();
			for (auto s : servers) {
				QStringList themes = s->getColorThemeProvider()->getSupportedThemes();
				if (not themes.isEmpty()) {
					langServerList.append(s);
				}
			}
			if (langServerList.isEmpty()) {
				return false;
			}
			currentLangServer = langServerList[0];
			for (auto s : langServerList) {
				langServerComboBox->addItem(s->getModuleName(), s->getModuleID());
			}
			langServerComboBox->setCurrentIndex(0);
			return true;
		}

		void onLangServerChanged(int index) {
			if (themeStyleDataEditingChanged) {
				int ret = QMessageBox::question(nullptr, VITR("YSS::colorThemeSettings.unsavedChanges"), VITR("YSS::colorThemeSettings.unsavedChangesPrompt"), QMessageBox::Yes | QMessageBox::No);
				if (ret == QMessageBox::Yes) {
					auto server = langServerList[index];
					currentLangServer->getColorThemeProvider()->setThemeStyleData(currentThemeName, themeStyleDataEditing);
				}
				themeStyleDataEditingChanged = false;
			}

			currentLangServer = langServerList[index];
			QStringList themes = currentLangServer->getColorThemeProvider()->getSupportedThemes();
			themeComboBox->clear();
			themeComboBox->addItems(themes);

			// 显示当前正在使用的主题
			int currentIndex = themeComboBox->findText(currentLangServer->getColorThemeProvider()->getCurrentTheme());
			themeComboBox->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);

			QString previewTextPath = currentLangServer->getColorThemeProvider()->getTemplateTextPath();
			if (not previewTextPath.isEmpty()) {
				documentPreviewTextEdit->openFile(previewTextPath);
			}
		}

		void onThemeChanged(int index) {
			if (themeStyleDataEditingChanged) {
				int ret = QMessageBox::question(nullptr, VITR("YSS::colorThemeSettings.unsavedChanges.title"), VITR("YSS::colorThemeSettings.unsavedChanges.desc"), QMessageBox::Yes | QMessageBox::No);
				if (ret == QMessageBox::Yes) {
					currentLangServer->getColorThemeProvider()->setThemeStyleData(currentThemeName, themeStyleDataEditing);
				}
				themeStyleDataEditingChanged = false;
			}
			currentThemeName = themeComboBox->itemText(index);
			themeStyleDataBackup = currentLangServer->getColorThemeProvider()->getThemeStyleData(currentThemeName);
			themeStyleDataEditing = themeStyleDataBackup;
			if (currentLangServer->getColorThemeProvider()->isStaticTheme(currentThemeName)) {
				staticWarningLabel->setVisible(true);
				duplicateButton->setEnabled(true);
				deleteButton->setEnabled(false);
			}
			else {
				staticWarningLabel->setVisible(false);
				duplicateButton->setEnabled(true);
				deleteButton->setEnabled(true);
			}

			auto* configModel = qobject_cast<QStandardItemModel*>(configNodeView->model());
			configModel->clear();
			for (auto key : themeStyleDataEditing.keys()) {
				auto styleData = themeStyleDataEditing[key];
				QString styleName = VI18N(styleData.getStyleName());
				auto* item = new QStandardItem(styleName);
				item->setData(key, Qt::UserRole);
				configModel->appendRow(item);
			}
			configNodeView->setCurrentIndex(configModel->index(0, 0));
			onConfigNodeSelected(configModel->index(0, 0));

			if (documentPreviewTextEdit->getSyntaxHighlighter()) {
				documentPreviewTextEdit->getSyntaxHighlighter()->onThemeChanged(themeStyleDataEditing);
				documentPreviewTextEdit->getSyntaxHighlighter()->rehighlight_s();
			}
		}

		QString currentStyleKey() const {
			auto index = configNodeView->currentIndex();
			if (not index.isValid()) {
				return QString();
			}
			return configNodeView->model()->data(index, Qt::UserRole).toString();
		}

		void onConfigNodeSelected(const QModelIndex& index) {
			if (not index.isValid()) {
				return;
			}
			QString styleKey = configNodeView->model()->data(index, Qt::UserRole).toString();
			if (styleKey.isEmpty() || not themeStyleDataEditing.contains(styleKey)) {
				return;
			}
			auto styleData = themeStyleDataEditing[styleKey];
			setColorButtonColor(textColorButton, styleData.getTextColor());
			setColorButtonColor(bgColorButton, styleData.getBgColor());
			setColorButtonColor(lineColorButton, styleData.getLineColor());
			updatingControls = true;
			lineTypeComboBox->setCurrentIndex(static_cast<int>(styleData.getUnderlineType()));
			useBgColorCheckBox->setChecked(styleData.isBgColorEnabled());
			useBoldCheckBox->setChecked(styleData.isBold());
			useItalicCheckBox->setChecked(styleData.isItalic());
			updatingControls = false;

			applyStyleToPreviewTextEdit();
		}

		void onAnyStylePropertyChanged() {
			if (updatingControls) {
				return;
			}
			QString styleKey = currentStyleKey();
			if (styleKey.isEmpty() || not themeStyleDataEditing.contains(styleKey)) {
				return;
			}
			auto& styleData = themeStyleDataEditing[styleKey];
			styleData.setTextColor(textColorButton->palette().button().color());
			styleData.setBgColor(bgColorButton->palette().button().color());
			styleData.setLineColor(lineColorButton->palette().button().color());
			styleData.setUnderlineType(static_cast<YSSCore::Editor::StyleData::UnderlineType>(lineTypeComboBox->currentIndex()));
			styleData.setBgColorEnabled(useBgColorCheckBox->isChecked());
			styleData.setBold(useBoldCheckBox->isChecked());
			styleData.setItalic(useItalicCheckBox->isChecked());
			themeStyleDataEditingChanged = true;
			if (documentPreviewTextEdit->getSyntaxHighlighter()) {
				documentPreviewTextEdit->getSyntaxHighlighter()->onThemeChanged(themeStyleDataEditing);
				documentPreviewTextEdit->getSyntaxHighlighter()->rehighlight_s();
			}
			applyStyleToPreviewTextEdit();
		}

		static void setColorButtonColor(QPushButton* button, const QColor& color) {
			QPalette pal = button->palette();
			pal.setColor(QPalette::Button, color);
			button->setPalette(pal);
		}

		void pickColorForButton(QPushButton* button) {
			QColor initial = button->palette().button().color();
			QColor color = QColorDialog::getColor(initial, button->window());
			if (not color.isValid()) {
				return;
			}
			setColorButtonColor(button, color);
			onAnyStylePropertyChanged();
		}

		void applyStyleToPreviewTextEdit() {
			QString styleKey = currentStyleKey();
			if (styleKey.isEmpty() || not themeStyleDataEditing.contains(styleKey)) {
				return;
			}
			const auto& styleData = themeStyleDataEditing.value(styleKey);
			QTextCharFormat format;
			format.setForeground(styleData.getTextColor());
			if (styleData.isBgColorEnabled()) {
				format.setBackground(styleData.getBgColor());
			}
			if (styleData.getUnderlineType() != YSSCore::Editor::StyleData::UnderlineType::NoUnderline) {
				format.setUnderlineColor(styleData.getLineColor());
				format.setUnderlineStyle(QTextCharFormat::UnderlineStyle(styleData.getUnderlineType()));
			}
			format.setFontWeight(styleData.isBold() ? QFont::Bold : QFont::Normal);
			format.setFontItalic(styleData.isItalic());

			QTextCursor cursor(previewTextEdit->document());
			cursor.select(QTextCursor::Document);
			cursor.setCharFormat(format);
		}

		void onSaveButtonClicked() {
			if (currentLangServer) {
				currentLangServer->getColorThemeProvider()->setThemeStyleData(currentThemeName, themeStyleDataEditing);
				// 保存当前选择的主题（切换）
				currentLangServer->getColorThemeProvider()->setCurrentTheme(currentThemeName);
				themeStyleDataEditingChanged = false;
			}
		}

		void onResetButtonClicked() {
			if (currentLangServer) {
				themeStyleDataEditing = themeStyleDataBackup;
				themeStyleDataEditingChanged = false;
				onConfigNodeSelected(configNodeView->currentIndex());
				if (documentPreviewTextEdit->getSyntaxHighlighter()) {
					documentPreviewTextEdit->getSyntaxHighlighter()->onThemeChanged(themeStyleDataEditing);
					documentPreviewTextEdit->getSyntaxHighlighter()->rehighlight_s();
				}
			}
		}
	};

	ColorThemeSettingsEditWidget::ColorThemeSettingsEditWidget(QWidget* parent) : YSSCore::Editor::FileEditWidget(parent) {
		d = new ColorThemeSettingsEditWidgetPrivate;

		// main layout
		d->mainLayout = new QGridLayout(this);
		d->mainLayout->setContentsMargins(8, 8, 8, 8);
		d->mainLayout->setSpacing(8);
		d->mainLayout->setColumnStretch(0, 3);
		d->mainLayout->setColumnStretch(1, 2);

		// style section (horizontal)
		d->styleFrame = new Visindigo::Widgets::BorderFrame(this);
		d->styleLayout = new QGridLayout(d->styleFrame);
		d->styleLayout->setContentsMargins(8, 4, 8, 4);
		d->fontTitleLabel = new QLabel(VITR("YSS::colorThemeSettings.font"), d->styleFrame);
		d->fontComboBox = new QFontComboBox(d->styleFrame);
		d->fontComboBox->setCurrentFont(TextEditConfigOperator::getTextFont());
		d->styleLayout->addWidget(d->fontTitleLabel, 0, 0);
		d->styleLayout->addWidget(d->fontComboBox, 0, 1);
		d->styleLayout->setColumnStretch(1, 1);

		// horizontal separator
		QFrame* separator = new QFrame(this);
		separator->setFrameShape(QFrame::HLine);
		separator->setFrameShadow(QFrame::Sunken);

		// langServer section (horizontal)
		d->langServerFrame = new Visindigo::Widgets::BorderFrame(this);
		d->langServerLayout = new QGridLayout(d->langServerFrame);
		d->langServerLayout->setContentsMargins(8, 4, 8, 4);
		d->langServerTitleLabel = new QLabel(VITR("YSS::colorThemeSettings.langServer"), d->langServerFrame);
		d->langServerComboBox = new QComboBox(d->langServerFrame);
		d->langServerLayout->addWidget(d->langServerTitleLabel, 0, 0);
		d->langServerLayout->addWidget(d->langServerComboBox, 0, 1);
		d->langServerLayout->setColumnStretch(1, 1);

		// theme section (horizontal)
		d->themeFrame = new Visindigo::Widgets::BorderFrame(this);
		d->themeLayout = new QGridLayout(d->themeFrame);
		d->themeLayout->setContentsMargins(8, 4, 8, 4);
		d->themeTitleLabel = new QLabel(VITR("YSS::colorThemeSettings.theme"), d->themeFrame);
		d->themeComboBox = new QComboBox(d->themeFrame);
		d->duplicateButton = new QPushButton(VITR("YSS::colorThemeSettings.duplicate"), d->themeFrame);
		d->deleteButton = new QPushButton(VITR("YSS::colorThemeSettings.delete"), d->themeFrame);
		d->themeLayout->addWidget(d->themeTitleLabel, 0, 0);
		d->themeLayout->addWidget(d->themeComboBox, 0, 1);
		d->themeLayout->addWidget(d->duplicateButton, 0, 2);
		d->themeLayout->addWidget(d->deleteButton, 0, 3);
		d->themeLayout->setColumnStretch(1, 1);

		// document preview (whole theme, powered by the language server highlighter)
		d->documentPreviewTextEdit = new YSSCore::Editor::TextEdit(this);
		d->documentPreviewTextEdit->setReadOnly(true);
		// 预览文档字体与全局编辑器字体保持一致
		d->documentPreviewTextEdit->setFont(TextEditConfigOperator::getTextFont());

		// edit section (grid)
		d->editFrame = new Visindigo::Widgets::BorderFrame(this);
		d->editLayout = new QGridLayout(d->editFrame);
		d->editLayout->setContentsMargins(8, 4, 8, 4);
		d->previewTextEdit = new QTextEdit(d->editFrame);
		d->previewTextEdit->setPlainText("ij = I::oO(0xB81l);");
		d->previewTextEdit->setReadOnly(true);
		d->previewTextEdit->setAlignment(Qt::AlignCenter);
		d->previewTextEdit->setFrameStyle(QFrame::NoFrame);
		auto target = TextEditConfigOperator::getTextFont();
		target.setPointSize(target.pointSize() * 1.5);
		d->previewTextEdit->setFont(target);
		auto height = std::max(30, d->previewTextEdit->fontMetrics().height() + 8);
		d->previewTextEdit->setFixedHeight(height);
		
		d->previewTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->previewTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->configNodeView = new QListView(d->editFrame);
		d->configNodeView->setSelectionMode(QAbstractItemView::SingleSelection);
		d->configNodeView->setModel(new QStandardItemModel(d->configNodeView));
		d->textColorTitleLabel = new QLabel(VITR("YSS::colorThemeSettings.textColor"), d->editFrame);
		d->textColorButton = new QPushButton(d->editFrame);
		d->bgColorTitleLabel = new QLabel(VITR("YSS::colorThemeSettings.bgColor"), d->editFrame);
		d->bgColorButton = new QPushButton(d->editFrame);
		d->lineColorTitleLabel = new QLabel(VITR("YSS::colorThemeSettings.lineColor"), d->editFrame);
		d->lineColorButton = new QPushButton(d->editFrame);
		d->lineTypeTitleLabel = new QLabel(VITR("YSS::colorThemeSettings.lineType"), d->editFrame);
		d->lineTypeComboBox = new QComboBox(d->editFrame);

		QMetaEnum underlineMeta = QMetaEnum::fromType<YSSCore::Editor::StyleData::UnderlineType>();
		for (int i = 0; i < underlineMeta.keyCount(); ++i) {
			d->lineTypeComboBox->addItem(VITR("YSS::colorThemeSettings.underlineType." + QString::fromUtf8(underlineMeta.key(i))));
		}
		
		d->useBoldCheckBox = new QCheckBox(VITR("YSS::colorThemeSettings.bold"), d->editFrame);
		d->useItalicCheckBox = new QCheckBox(VITR("YSS::colorThemeSettings.italic"), d->editFrame);
		d->useBgColorCheckBox = new QCheckBox(VITR("YSS::colorThemeSettings.bgColorEnabled"), d->editFrame);
		// row 0: configNodeView (full row)
		d->editLayout->addWidget(d->configNodeView, 0, 0, 1, -1);
		// row 1: per-style preview (single line, full row)
		d->editLayout->addWidget(d->previewTextEdit, 1, 0, 1, -1);
		// rows 2-6: left title, right setting
		d->editLayout->addWidget(d->textColorTitleLabel, 2, 0);
		d->editLayout->addWidget(d->textColorButton, 2, 1);
		d->editLayout->addWidget(d->useBgColorCheckBox, 3, 0, 1, -1);
		d->editLayout->addWidget(d->bgColorTitleLabel, 4, 0);
		d->editLayout->addWidget(d->bgColorButton, 4, 1);
		d->editLayout->addWidget(d->lineColorTitleLabel, 5, 0);
		d->editLayout->addWidget(d->lineColorButton, 5, 1);
		d->editLayout->addWidget(d->lineTypeTitleLabel, 6, 0);
		d->editLayout->addWidget(d->lineTypeComboBox, 6, 1);
		// last row: the two checkboxes in the same row
		d->editLayout->addWidget(d->useBoldCheckBox, 7, 0);
		d->editLayout->addWidget(d->useItalicCheckBox, 7, 1);
		d->editLayout->setColumnStretch(0, 1);
		d->editLayout->setColumnStretch(1, 1);

		// save / reset buttons
		d->staticWarningLabel = new QLabel(VITR("YSS::colorThemeSettings.staticThemeWarning"), this);
		d->saveButton = new QPushButton(VITR("YSS::colorThemeSettings.save"), this);
		d->resetButton = new QPushButton(VITR("YSS::colorThemeSettings.reset"), this);

		// mainLayout arrangement
		d->mainLayout->addWidget(d->styleFrame, 0, 0, 1, -1);
		d->mainLayout->addWidget(separator, 1, 0, 1, -1);
		d->mainLayout->addWidget(d->langServerFrame, 2, 0, 1, -1);
		d->mainLayout->addWidget(d->themeFrame, 3, 0, 1, -1);
		d->mainLayout->addWidget(d->documentPreviewTextEdit, 4, 0);
		d->mainLayout->addWidget(d->editFrame, 4, 1, 1, -1);
		d->mainLayout->addWidget(d->staticWarningLabel, 5, 0);
		d->mainLayout->addWidget(d->saveButton, 5, 1, Qt::AlignRight);
		d->mainLayout->addWidget(d->resetButton, 5, 2, Qt::AlignRight);
		d->mainLayout->setRowStretch(4, 1);

		connect(d->langServerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
			d->onLangServerChanged(index);
			});
		connect(d->themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
			d->onThemeChanged(index);
			});
		connect(d->fontComboBox, &QFontComboBox::currentFontChanged, this, [this](const QFont& font) {
			d->documentPreviewTextEdit->setFont(font);
			TextEditConfigOperator::setTextFont(font, true);
			auto target = font;
			target.setPointSize(font.pointSize() * 1.5);
			d->previewTextEdit->setFont(target);
			auto height = std::max(30, d->previewTextEdit->fontMetrics().height() + 8);
			d->previewTextEdit->setFixedHeight(height);
			});
		connect(d->configNodeView, &QListView::clicked, this, [this](const QModelIndex& index) {
			d->onConfigNodeSelected(index);
			});
		connect(d->textColorButton, &QPushButton::clicked, this, [this]() {
			d->pickColorForButton(d->textColorButton);
			});
		connect(d->bgColorButton, &QPushButton::clicked, this, [this]() {
			d->pickColorForButton(d->bgColorButton);
			});
		connect(d->lineColorButton, &QPushButton::clicked, this, [this]() {
			d->pickColorForButton(d->lineColorButton);
			});
		connect(d->lineTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
			d->onAnyStylePropertyChanged();
			});
		connect(d->useBoldCheckBox, &QCheckBox::stateChanged, this, [this](int) {
			d->onAnyStylePropertyChanged();
			});
		connect(d->useItalicCheckBox, &QCheckBox::stateChanged, this, [this](int) {
			d->onAnyStylePropertyChanged();
			});
		connect(d->useBgColorCheckBox, &QCheckBox::stateChanged, this, [this](int) {
			d->onAnyStylePropertyChanged();
			});
		connect(d->saveButton, &QPushButton::clicked, this, [this]() {
			d->onSaveButtonClicked();
			});
		connect(d->resetButton, &QPushButton::clicked, this, [this]() {
			d->onResetButtonClicked();
			});
		connect(d->duplicateButton, &QPushButton::clicked, this, [this]() {
			if (d->currentLangServer == nullptr || d->currentThemeName.isEmpty()) {
				return;
			}
			QString sourceThemeName = d->currentThemeName;
			auto* copyDialog = new ColorThemeSettingsCopyDialog(this);
			copyDialog->setAttribute(Qt::WA_DeleteOnClose);
			copyDialog->setCurrentThemes(d->currentLangServer->getColorThemeProvider()->getSupportedThemes());
			copyDialog->setFromThemeName(sourceThemeName);
			connect(copyDialog, &ColorThemeSettingsCopyDialog::confirmed, this, [this, sourceThemeName](const QString& newName) {
				auto* provider = d->currentLangServer->getColorThemeProvider();
				provider->createNewTheme(newName, sourceThemeName);
				// 刷新主题下拉框并选中新主题
				QStringList themes = provider->getSupportedThemes();
				d->themeComboBox->clear();
				d->themeComboBox->addItems(themes);
				int idx = d->themeComboBox->findText(newName);
				if (idx >= 0) {
					d->themeComboBox->setCurrentIndex(idx);
				}
				});
			copyDialog->show();
			});
		connect(d->deleteButton, &QPushButton::clicked, this, [this]() {
			if (d->currentLangServer == nullptr || d->currentThemeName.isEmpty()) {
				return;
			}
			auto* provider = d->currentLangServer->getColorThemeProvider();
			if (provider->isStaticTheme(d->currentThemeName)) {
				return; // 静态主题不允许删除
			}
			int ret = QMessageBox::question(this, VITR("YSS::colorThemeSettings.deleteConfirm.title"),
				VITR("YSS::colorThemeSettings.deleteConfirm.message").arg(d->currentThemeName),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
			if (ret != QMessageBox::Yes) {
				return;
			}
			provider->removeTheme(d->currentThemeName);
			// 刷新主题下拉框，选中第一个可用的主题
			QStringList themes = provider->getSupportedThemes();
			d->themeComboBox->clear();
			d->themeComboBox->addItems(themes);
			d->themeComboBox->setCurrentIndex(0);
			});
		bool success = d->initLangServer();

		d->nothingToEditLabel = new QLabel(VITR("YSS::colorThemeSettings.nothingToEdit"), this);
		d->nothingToEditLabel->setAlignment(Qt::AlignCenter);
		d->nothingToEditLabel->setStyleSheet("QLabel{color: gray; font-size: 16px;}");
		if (not success) {
			d->nothingToEditLabel->show();
		}
		else {
			d->nothingToEditLabel->hide();
		}
	}

	ColorThemeSettingsEditWidget::~ColorThemeSettingsEditWidget() {
		delete d;
	}

	void ColorThemeSettingsEditWidget::resizeEvent(QResizeEvent* event) {
		QWidget::resizeEvent(event);
		d->nothingToEditLabel->setGeometry(0, 0, width(), height());
	}

	bool ColorThemeSettingsEditWidget::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		if (ext == "YSS.MainEditor.ColorThemeSettings") {
			return true;
		}
		return false;
	}

	bool ColorThemeSettingsEditWidget::onVirtualClose() {
		return true;
	}

	bool ColorThemeSettingsEditWidget::onVirtualSave() {
		return true;
	}
}