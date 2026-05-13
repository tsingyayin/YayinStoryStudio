#include "Editor/MainEditor/private/StackComponents_p.h"
#include <QtCore/qfileinfo.h>
#include <Editor/DocumentMessageManager.h>
#include <General/TranslationHost.h>
#include <Editor/SyntaxHighlighter.h>
#include <General/Log.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qscrollbar.h>
#include <Utility/FileUtility.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServerManager.h>
namespace YSS::Editor {
	StackTag::StackTag(QWidget* parent) :QFrame(parent) {
		TitleLabel = new QLabel(this);
		PinLabel = new QPushButton(this);
		PinLabel->setIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon/pin.png"));
		CloseLabel = new QPushButton(this);
		CloseLabel->setIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon/close.png"));
		Layout = new QHBoxLayout(this);
		Layout->setContentsMargins(5, 0, 5, 0);
		Layout->addWidget(TitleLabel);
		Layout->addWidget(PinLabel);
		Layout->addWidget(CloseLabel);
		//Layout->addWidget(Separator);

		PinLabel->setFixedWidth(PinLabel->height());
		CloseLabel->setFixedWidth(CloseLabel->height());
		PinLabel->hide();
		CloseLabel->hide();

		this->setMaximumWidth(200);
		connect(PinLabel, &QPushButton::clicked, this, [this]() {
			setPinned(!isPinned());
			emit pinClicked(FilePath);
			});

		connect(CloseLabel, &QPushButton::clicked, this, [this]() {
			emit closeClicked(FilePath);
			});

		setContextMenuPolicy(Qt::ActionsContextMenu);
		ActionClose = new QAction(VITR("Visindigo::general.close"), this);
		ActionPin = new QAction(VITR("Visindigo::general.pin"), this);
		ActionReload = new QAction(VITR("Visindigo::general.reload"), this);
		ActionRename = new QAction(VITR("Visindigo::general.rename"), this);
		ActionSaveAs = new QAction(VITR("Visindigo::general.saveAs"), this);
		ActionShowInExplorer = new QAction(VITR("YSS::menu.file.showInExplorer"), this);
		ActionCloseAll = new QAction(VITR("YSS::menu.file.closeAll"), this);
		ActionCloseSaved = new QAction(VITR("YSS::menu.file.closeSaved"), this);
		connect(ActionClose, &QAction::triggered, this, [this]() {
			emit closeClicked(FilePath);
			});
		connect(ActionPin, &QAction::triggered, this, [this]() {
			setPinned(!isPinned());
			emit pinClicked(FilePath);
			});
		connect(ActionReload, &QAction::triggered, this, [this]() {
			YSSCore::Editor::FileEditWidget* editor = YSSFSM->getFileEditWidget(FilePath);
			if (editor) { 
				editor->reloadFile();
			}
			});
		connect(ActionRename, &QAction::triggered, this, [this]() {
			emit renameRequested(FilePath);
			});
		connect(ActionSaveAs, &QAction::triggered, this, [this]() {
			emit saveAsRequested(FilePath);
			});
		connect(ActionShowInExplorer, &QAction::triggered, this, [this]() {
			Visindigo::Utility::FileUtility::openExplorer(FilePath);
			});
		connect(ActionCloseAll, &QAction::triggered, this, [this]() {
			emit closeAllRequested();
			});
		connect(ActionCloseSaved, &QAction::triggered, this, [this]() {
			emit closeSavedRequested();
			});
		this->addActions({ ActionClose, ActionPin, ActionReload, 
			ActionRename, ActionSaveAs, ActionShowInExplorer,
			ActionCloseAll, ActionCloseSaved });
	}

	void StackTag::setText(const QString& text) {
		TitleLabel->setText(text);
	}

	void StackTag::setFilePath(const QString& filePath) {
		FilePath = filePath;
	}

	QString StackTag::getFilePath() const {
		return FilePath;
	}

	QString StackTag::getText() const { 
		return TitleLabel->text();
	}

	void StackTag::setFocusOn(bool focus) {
		Focused = focus;
		if (Focused) {
			QFrame::setStyleSheet(PressedStyle);
		}
		else {
			QFrame::setStyleSheet(NormalStyle);
		}
	}

	bool StackTag::isFocusOn() const {
		return Focused;
	}

	void StackTag::setPinned(bool pinned) {
		if (Pinned == pinned) {
			return;
		}
		Pinned = pinned;
		if (Pinned) {
			PinLabel->show();
		}
		else {
			PinLabel->hide();
		}
	}

	bool StackTag::isPinned() const {
		return Pinned;
	}

	void StackTag::setStyleSheet(const QString& normal, const QString& hover, const QString& pressed) {
		NormalStyle = normal;
		HoverStyle = hover;
		PressedStyle = pressed;
		if (Focused) {
			QFrame::setStyleSheet(PressedStyle);
		}
		else {
			QFrame::setStyleSheet(NormalStyle);
		}
	}

	void StackTag::mousePressEvent(QMouseEvent* event) {
		QFrame::mousePressEvent(event);
		if (event->button() == Qt::LeftButton) {
			Pressed = true;
			QFrame::setStyleSheet(PressedStyle);
		}
	}

	void StackTag::mouseReleaseEvent(QMouseEvent* event) {
		QFrame::mouseReleaseEvent(event);
		if (Pressed && event->button() == Qt::LeftButton) {
			Pressed = false;
			if (Focused) {
				QFrame::setStyleSheet(PressedStyle);
			}
			else {
				QFrame::setStyleSheet(HoverStyle);
			}
			emit clicked(FilePath);
		}
	}

	void StackTag::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		PinLabel->setFixedWidth(PinLabel->height());
		CloseLabel->setFixedWidth(CloseLabel->height());
		QFontMetrics fm(TitleLabel->font());
		int textWidth = fm.horizontalAdvance(TitleLabel->text());
		if (textWidth > this->TitleLabel->width()) {
			this->setToolTip(TitleLabel->text());
		}
		else {
			this->setToolTip(QString());
		}
	}

	void StackTag::enterEvent(QEnterEvent* event) {
		QFrame::enterEvent(event);
		if (not Focused) {
			QFrame::setStyleSheet(HoverStyle);
		}
		PinLabel->show();
		CloseLabel->show();
		QFontMetrics fm(TitleLabel->font());
		int textWidth = fm.horizontalAdvance(TitleLabel->text());
		if (textWidth > this->TitleLabel->width()) {
			this->setToolTip(TitleLabel->text());
		}
		else {
			this->setToolTip(QString());
		}
	}

	void StackTag::leaveEvent(QEvent* event) {
		QFrame::leaveEvent(event);
		if (not Focused) {
			QFrame::setStyleSheet(NormalStyle);
		}
		if (not Pinned) {
			PinLabel->hide();
		}
		CloseLabel->hide();
		QFontMetrics fm(TitleLabel->font());
		int textWidth = fm.horizontalAdvance(TitleLabel->text());
		if (textWidth > this->TitleLabel->width()) {
			this->setToolTip(TitleLabel->text());
		}
		else {
			this->setToolTip(QString());
		}
	}

	StackTagWidget::StackTagWidget(QWidget* parent) :QFrame(parent) {
		ContentLayout = new QHBoxLayout();
		ContentLayout->setContentsMargins(0, 0, 0, 0);
		ContentLayout->setSpacing(2);
		ScrollContent = new QFrame();
		ScrollContent->setLayout(ContentLayout);
		ScrollArea = new QScrollArea(this);
		ScrollArea->setWidget(ScrollContent);
		ScrollArea->setWidgetResizable(true);
		ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		WidgetSelector = new QComboBox(this);
		WidgetSelector->setMinimumWidth(200);
		Layout = new QHBoxLayout(this);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->addWidget(ScrollArea);
		Layout->addWidget(WidgetSelector);
		connect(WidgetSelector, &QComboBox::currentIndexChanged, this, [this](int index) {
			QString filePath = WidgetSelector->itemData(index).toString();
			setCurrentStackLabel(filePath);
			emit switchToFile(filePath);
			});
		setColorfulEnable(true);
		onThemeChanged();
	}

	StackTagWidget::~StackTagWidget() {
	}

	void StackTagWidget::addStackLabel(const QString& filePath, const QString& displayName) {
		QFileInfo fileInfo(filePath);
		StackTag* tagLabel = new StackTag();
		tagLabel->setFilePath(filePath);
		if (displayName.isEmpty()) {
			tagLabel->setText(fileInfo.fileName());
			WidgetSelector->addItem(fileInfo.fileName(), filePath);
		}
		else {
			tagLabel->setText(displayName);
			WidgetSelector->addItem(displayName, filePath);
		}
		ContentLayout->addWidget(tagLabel);
		Labels.append(tagLabel);
		
		connect(tagLabel, &StackTag::pinClicked, this, [this](const QString& filePath) {
			pinStackLabel(filePath);
			});
		connect(tagLabel, &StackTag::closeClicked, this, [this](const QString& filePath) {
			emit closeFile(filePath); // NOTICE: close should be handled by some dialog, cannot directly do any other operation here.
			});
		connect(tagLabel, &StackTag::clicked, this, [this](const QString& filePath) {
			emit switchToFile(filePath);
			});
		connect(tagLabel, &StackTag::renameRequested, this, [this](const QString& filePath) {
			emit renameRequested(filePath);
			});
		connect(tagLabel, &StackTag::saveAsRequested, this, [this](const QString& filePath) {
			emit saveAsRequested(filePath);
			});
		connect(tagLabel, &StackTag::closeAllRequested, this, [this]() {
			emit closeAllRequested();
			});
		connect(tagLabel, &StackTag::closeSavedRequested, this, [this]() {
			emit closeSavedRequested();
			});
		ScrollArea->horizontalScrollBar()->setMaximum(Labels.size() * Labels.last()->width() - ScrollArea->width());
		tagLabel->setStyleSheet(VISTMGT("YSS::Editor.StackTag.Normal"),
			VISTMGT("YSS::Editor.StackTag.Hover"),
			VISTMGT("YSS::Editor.StackTag.Pressed"));
		adjustScrollArea();
	}

	void StackTagWidget::changeStackLabel(const QString& oldFilePath, const QString& newFilePath, const QString& newDisplayName) {
		for (StackTag* label : Labels) {
			if (label->getFilePath() == oldFilePath) {
				QFileInfo fileInfo(newFilePath);
				if (newDisplayName.isEmpty()) {
					label->setText(fileInfo.fileName());
				}
				else {
					label->setText(newDisplayName);
				}
				label->setFilePath(newFilePath);
				for (int i = 0; i < WidgetSelector->count(); ++i) {
					if (WidgetSelector->itemData(i).toString() == oldFilePath) {
						if (newDisplayName.isEmpty()) {
							WidgetSelector->setItemText(i, fileInfo.fileName());
						}
						else {
							WidgetSelector->setItemText(i, newDisplayName);
						}
						WidgetSelector->setItemData(i, newFilePath);
						break;
					}
				}
				break;
			}
		}
	}

	void StackTagWidget::pinStackLabel(const QString& filePath) {
		StackTag* targetLabel = nullptr;
		for (StackTag* label : Labels) {
			if (label->getFilePath() == filePath) {
				targetLabel = label;
			}
		}
		// NOTICE:
		// This move-afterward operation working in two situations:
		// 1. When pinning, it will move the label to the leftmost of all unpinned labels, 
		// which is the most intuitive way to do it.
		// 2. When unpinning, it will move the label to the rightmost of all pinned labels,
		// Fortunately, these two situations are actually the same operation, 
		// which is to move the label to the boundary of pinned and unpinned labels.
		if (targetLabel) {
			Labels.removeAll(targetLabel);
			ContentLayout->removeWidget(targetLabel);
			for (int i = 0; i < Labels.size(); ++i) {
				if (not Labels[i]->isPinned()) {
					Labels.insert(i, targetLabel);
					ContentLayout->insertWidget(i, targetLabel);
					return;
				}
			}
			// All pinned.
			Labels.append(targetLabel);
			ContentLayout->addWidget(targetLabel);
		}
	}

	void StackTagWidget::removeStackLabel(const QString& filePath) {
		StackTag* targetLabel = nullptr;
		int index = 0;
		for (int i = 0; i < Labels.size(); ++i) {
			if (Labels[i]->getFilePath() == filePath) {
				targetLabel = Labels[i];
				index = i;
				break;
			}
		}
		if (targetLabel) {
			Labels.removeAll(targetLabel);
			ContentLayout->removeWidget(targetLabel);
			targetLabel->deleteLater();
			for (int i = 0; i < WidgetSelector->count(); ++i) {
				if (WidgetSelector->itemData(i).toString() == filePath) {
					WidgetSelector->removeItem(i);
					break;
				}
			}
		}
		if (index >1){
			if (index - 1 < Labels.size()) {
				auto label = Labels[index];
				if (label) {
					setCurrentStackLabel(label->getFilePath());
					emit switchToFile(label->getFilePath());
				}
			}
			else {
				CurrentSelected = "";
				emit switchToFile("");	
			}
		}
		else {
			if (Labels.size() > 0) {
				auto label = Labels[0];
				if (label) {
					setCurrentStackLabel(label->getFilePath());
					emit switchToFile(label->getFilePath());
				}
			}
			else {
				CurrentSelected = "";
				emit switchToFile("");
			}
		}
		adjustScrollArea();
	}

	void StackTagWidget::setCurrentStackLabel(const QString& filePath) {
		if (filePath == CurrentSelected) {
			return;
		}
		bool finded = false;
		int i = 0;
		int cache = i;
		for (StackTag* label : Labels) {
			if (label->getFilePath() == filePath) {
				label->setFocusOn(true);
				finded = true;
				CurrentSelected = filePath;
				cache = i;
			}
			else {
				label->setFocusOn(false);
			}
			i++;
		}
		if (finded) {
			for (int i = 0; i < WidgetSelector->count(); ++i) {
				if (WidgetSelector->itemData(i).toString() == filePath) {
					WidgetSelector->setCurrentIndex(i);
					break;
				}
			}
			ScrollArea->horizontalScrollBar()->setValue(cache * Labels.last()->width());
		}
	}

	QString StackTagWidget::getCurrentSelected() const {
		return CurrentSelected;
	}

	void StackTagWidget::adjustScrollArea() {
		if (Labels.size() == 0) {
			ScrollContent->setFixedWidth(0);
			return;
		}
		int totalWidth = 0;
		for (StackTag* label : Labels) {
			totalWidth += label->width() + ContentLayout->spacing();
		}
		totalWidth -= ContentLayout->spacing();
		ScrollContent->setFixedWidth(totalWidth + 2 * ScrollContent->frameWidth());
	}

	void StackTagWidget::setFileChanged(const QString& path) {
		for (auto label : Labels) {
			if (label->getFilePath() == path) {
				QString fileName = label->getText();
				if (not fileName.startsWith("* ")) {
					label->setText("* " + fileName);
				}
				break;
			}
		}
	}
	
	void StackTagWidget::cancelFileChanged(const QString& path) {
		for (auto label : Labels) {
			if (label->getFilePath() == path) {
				QString fileName = label->getText();
				if (fileName.startsWith("* ")) {
					label->setText(fileName.mid(2));
				}
				break;
			}
		}
	}

	bool StackTagWidget::containsStackLabel(const QString& filePath) const {
		for (auto label : Labels) {
			if (label->getFilePath() == filePath) {
				return true;
			}
		}
		return false;
	}

	bool StackTagWidget::isStackLabelPinned(const QString& filePath) const {
		for (auto label : Labels) {
			if (label->getFilePath() == filePath) {
				return label->isPinned();
			}
		}
		return false;
	}

	void StackTagWidget::wheelEvent(QWheelEvent* event) {
		QFrame::wheelEvent(event);
		int numDegrees = event->angleDelta().y() / 8;
		int numSteps = numDegrees / 15;
		int stepSize = Labels.size() > 0 ? Labels[0]->width() + ContentLayout->spacing() : 100;
		ScrollArea->horizontalScrollBar()->setValue(ScrollArea->horizontalScrollBar()->value() - numSteps * stepSize);
	}

	void StackTagWidget::onThemeChanged() {
		for (StackTag* label : Labels) {
			label->setStyleSheet(VISTMGT("YSS::Editor.StackTag.Normal"),
				VISTMGT("YSS::Editor.StackTag.Hover"),
				VISTMGT("YSS::Editor.StackTag.Pressed"));
		}
	}
	void StackTagWidget::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		adjustScrollArea();
	}

	MessageViewer::MessageViewer(QWidget* parent) :QFrame(parent) {
		MessageTable = new QTableWidget(this);
		MessageTable->setColumnCount(5); // code, message, file, line, column
		MessageTable->setHorizontalHeaderLabels({
				VITR("YSS::editor.messageViewer.code"),
				VITR("YSS::editor.messageViewer.message"),
				VITR("YSS::editor.messageViewer.file"),
				VITR("YSS::editor.messageViewer.line"),
				VITR("YSS::editor.messageViewer.column")
			});
		Layout = new QVBoxLayout(this);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->addWidget(MessageTable);
		MessageTable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
		MessageTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
		MessageTable->setColumnWidth(0, 100);
		MessageTable->setColumnWidth(3, 100);
		MessageTable->setColumnWidth(4, 100);
		MessageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
		connect(YSSCore::Editor::DocumentMessageManager::getInstance(), 
			&YSSCore::Editor::DocumentMessageManager::messageChanged, this, &MessageViewer::onMessageChanged);
		connect(YSSCore::Editor::DocumentMessageManager::getInstance(),
			&YSSCore::Editor::DocumentMessageManager::messageChangedForLine, this, &MessageViewer::onMessageChangedForLine);
		connect(MessageTable, &QTableWidget::cellClicked, this, &MessageViewer::onCellClicked);
	}

	void MessageViewer::changeCurrentFile(const QString& filePath) {
		if (CurrentFilePath == filePath) {
			return;
		}
		CurrentFilePath = filePath;
		MessageTable->clearContents();
		MessageTable->setRowCount(0);
		onMessageChanged(filePath);
	}

	void MessageViewer::onCellClicked(int row, int column) {
		QString filePath = MessageTable->item(row, 2)->toolTip();
		qint32 lineNumber = MessageTable->item(row, 3)->text().toInt() - 1;
		qint32 columnNumber = MessageTable->item(row, 4)->text().toInt();
		//vgDebug << "Redirection requested to " << filePath << ":" << lineNumber << ":" << columnNumber;
		emit YSSCore::Editor::FileServerManager::getInstance()->focusOnFile(filePath, lineNumber, columnNumber);
	}

	void MessageViewer::onMessageChanged(const QString& filePath) {
		//vgDebug << "Message changed for file: " << filePath;
		if (filePath == CurrentFilePath) {
			MessageTable->clearContents();
			MessageTable->setRowCount(0);
			auto messages = YSSCore::Editor::DocumentMessageManager::getInstance()->getAllMessages(filePath);
			qint32 totalLines = 0;
			for (auto msgList : messages.values()) {
				totalLines += msgList.size();
			}
			MessageTable->setRowCount(totalLines);
			qint32 row = 0;
			for (auto msgList : messages.values()) {
				for (auto msg : msgList) {
					auto msgCode = new QLabel();
					msgCode->setText(QString("<a href=\"%1\">%2</a>").arg(msg.getHelpUrl().toString()).arg(msg.getCode()));
					msgCode->setOpenExternalLinks(true);
					msgCode->setAlignment(Qt::AlignCenter);
					MessageTable->setCellWidget(row, 0, msgCode);
					auto messageItem = new QTableWidgetItem(msg.getMessage());
					messageItem->setToolTip(msg.getFixAdvice());
					MessageTable->setItem(row, 1, messageItem);
					auto filePathItem = new QTableWidgetItem(QFileInfo(filePath).fileName());
					filePathItem->setToolTip(filePath);
					MessageTable->setItem(row, 2, filePathItem);
					auto lineItem = new QTableWidgetItem(QString::number(msg.getLineNumber() + 1));
					lineItem->setTextAlignment(Qt::AlignCenter);
					MessageTable->setItem(row, 3, lineItem);
					auto columnItem = new QTableWidgetItem(QString::number(msg.getColumnNumber()));
					columnItem->setTextAlignment(Qt::AlignCenter);
					MessageTable->setItem(row, 4, columnItem);
					row++;
				}
			}
		}
	}

	void MessageViewer::onMessageChangedForLine(const QString& filePath, qint32 lineNumber) {
		// search all line == lineNumber, remove it
		//vgDebug << filePath << CurrentFilePath;
		if (filePath != CurrentFilePath) {
			return;
		}
		for (int i = 0; i < MessageTable->rowCount(); ++i) {
			if (MessageTable->item(i, 2)->toolTip() == filePath && MessageTable->item(i, 3)->text().toInt() == lineNumber+1) {
				MessageTable->removeRow(i);
				//vgDebug << "Removed message for line " << lineNumber << " at row " << i;
				--i;
			}
		}
		auto msgList = YSSCore::Editor::DocumentMessageManager::getInstance()->getMessages(filePath, lineNumber);
		//vgDebug << "Line " << lineNumber << " has " << msgList.size() << " messages.";
		for (auto msg : msgList) {
			int row = MessageTable->rowCount();
			MessageTable->insertRow(row);
			auto msgCode = new QLabel();
			msgCode->setText(QString("<a href=\"%1\">%2</a>").arg(msg.getHelpUrl().toString()).arg(msg.getCode()));
			msgCode->setOpenExternalLinks(true);
			msgCode->setAlignment(Qt::AlignCenter);
			MessageTable->setCellWidget(row, 0, msgCode);
			auto messageItem = new QTableWidgetItem(msg.getMessage());
			messageItem->setToolTip(msg.getFixAdvice());
			MessageTable->setItem(row, 1, messageItem);
			auto filePathItem = new QTableWidgetItem(QFileInfo(filePath).fileName());
			filePathItem->setToolTip(filePath);
			MessageTable->setItem(row, 2, filePathItem);
			auto lineItem = new QTableWidgetItem(QString::number(msg.getLineNumber() + 1));
			lineItem->setTextAlignment(Qt::AlignCenter);
			MessageTable->setItem(row, 3, lineItem);
			auto columnItem = new QTableWidgetItem(QString::number(msg.getColumnNumber()));
			columnItem->setTextAlignment(Qt::AlignCenter);
			MessageTable->setItem(row, 4, columnItem);
		}
		if (msgList.size() != 0) {
			MessageTable->sortByColumn(3, Qt::AscendingOrder);
		}
	}

	DefaultStackWidgetCentralArea::DefaultStackWidgetCentralArea(QWidget* parent) :QFrame(parent) {
		ContentLabel = new QLabel(this);
		Layout = new QGridLayout(this);
		this->setLayout(Layout);
		Layout->addWidget(ContentLabel, 0, 0, Qt::AlignCenter);
		ContentLabel->setAlignment(Qt::AlignCenter);
	}

	void DefaultStackWidgetCentralArea::setText(const QString& text) {
		ContentLabel->setText(text);
	}
}