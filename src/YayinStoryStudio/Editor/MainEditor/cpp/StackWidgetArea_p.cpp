#include "Editor/MainEditor/private/StackWidgetArea_p.h"
#include <QtCore/qfileinfo.h>
#include <Editor/DocumentMessageManager.h>
#include <General/TranslationHost.h>
#include <Editor/SyntaxHighlighter.h>
#include <General/Log.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qscrollbar.h>
namespace YSS::Editor {
	StackWidgetTagLabel::StackWidgetTagLabel(QWidget* parent) :QFrame(parent) {
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
	}

	void StackWidgetTagLabel::setText(const QString& text) {
		TitleLabel->setText(text);
	}

	void StackWidgetTagLabel::setFilePath(const QString& filePath) {
		FilePath = filePath;
	}

	QString StackWidgetTagLabel::getFilePath() const {
		return FilePath;
	}

	QString StackWidgetTagLabel::getText() const { 
		return TitleLabel->text();
	}

	void StackWidgetTagLabel::setFocusOn(bool focus) {
		Focused = focus;
		if (Focused) {
			QFrame::setStyleSheet(PressedStyle);
		}
		else {
			QFrame::setStyleSheet(NormalStyle);
		}
	}

	bool StackWidgetTagLabel::isFocusOn() const {
		return Focused;
	}

	void StackWidgetTagLabel::setPinned(bool pinned) {
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

	bool StackWidgetTagLabel::isPinned() const {
		return Pinned;
	}

	void StackWidgetTagLabel::setStyleSheet(const QString& normal, const QString& hover, const QString& pressed) {
		NormalStyle = normal;
		HoverStyle = hover;
		PressedStyle = pressed;
		QFrame::setStyleSheet(NormalStyle);
	}

	void StackWidgetTagLabel::mousePressEvent(QMouseEvent* event) {
		QFrame::mousePressEvent(event);
		if (event->button() == Qt::LeftButton) {
			Pressed = true;
			QFrame::setStyleSheet(PressedStyle);
		}
	}

	void StackWidgetTagLabel::mouseReleaseEvent(QMouseEvent* event) {
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

	void StackWidgetTagLabel::resizeEvent(QResizeEvent* event) {
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

	void StackWidgetTagLabel::enterEvent(QEnterEvent* event) {
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

	void StackWidgetTagLabel::leaveEvent(QEvent* event) {
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

	StackWidgetTagArea::StackWidgetTagArea(QWidget* parent) :QFrame(parent) {
		ContentLayout = new QHBoxLayout();
		ContentLayout->setContentsMargins(0, 0, 0, 0);
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

	StackWidgetTagArea::~StackWidgetTagArea() {
	}

	void StackWidgetTagArea::addStackLabel(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		StackWidgetTagLabel* tagLabel = new StackWidgetTagLabel();
		tagLabel->setFilePath(filePath);
		tagLabel->setText(fileInfo.fileName());
		ContentLayout->addWidget(tagLabel);
		Labels.append(tagLabel);
		WidgetSelector->addItem(fileInfo.fileName(), filePath);
		connect(tagLabel, &StackWidgetTagLabel::pinClicked, this, [this](const QString& filePath) {
			pinStackLabel(filePath);
			});
		connect(tagLabel, &StackWidgetTagLabel::closeClicked, this, [this](const QString& filePath) {
			emit closeFile(filePath); // NOTICE: close should be handled by some dialog, cannot directly do any other operation here.
			});
		connect(tagLabel, &StackWidgetTagLabel::clicked, this, [this](const QString& filePath) {
			emit switchToFile(filePath);
			});
		ScrollArea->horizontalScrollBar()->setMaximum(Labels.size() * Labels.last()->width() - ScrollArea->width());
		tagLabel->setStyleSheet(VISTMGT("YSS::Editor.StackWidgetTag.Normal"),
			VISTMGT("YSS::Editor.StackWidgetTag.Hover"),
			VISTMGT("YSS::Editor.StackWidgetTag.Pressed"));
		adjustScrollArea();
	}

	void StackWidgetTagArea::pinStackLabel(const QString& filePath) {
		StackWidgetTagLabel* targetLabel = nullptr;
		for (StackWidgetTagLabel* label : Labels) {
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

	void StackWidgetTagArea::removeStackLabel(const QString& filePath) {
		StackWidgetTagLabel* targetLabel = nullptr;
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

	void StackWidgetTagArea::setCurrentStackLabel(const QString& filePath) {
		if (filePath == CurrentSelected) {
			return;
		}
		bool finded = false;
		int i = 0;
		int cache = i;
		for (StackWidgetTagLabel* label : Labels) {
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

	QString StackWidgetTagArea::getCurrentSelected() const {
		return CurrentSelected;
	}

	void StackWidgetTagArea::adjustScrollArea() {
		int totalWidth = 0;
		for (StackWidgetTagLabel* label : Labels) {
			totalWidth += label->width() + ContentLayout->spacing();
		}
		ScrollContent->setFixedWidth(totalWidth + 2 * ScrollContent->frameWidth());
	}

	void StackWidgetTagArea::setFileChanged(const QString& path) {
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
	
	void StackWidgetTagArea::cancelFileChanged(const QString& path) {
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

	void StackWidgetTagArea::wheelEvent(QWheelEvent* event) {
		QFrame::wheelEvent(event);
		int numDegrees = event->angleDelta().y() / 8;
		int numSteps = numDegrees / 15;
		int stepSize = Labels.size() > 0 ? Labels[0]->width() + ContentLayout->spacing() : 100;
		ScrollArea->horizontalScrollBar()->setValue(ScrollArea->horizontalScrollBar()->value() - numSteps * stepSize);
	}

	void StackWidgetTagArea::onThemeChanged() {
		for (StackWidgetTagLabel* label : Labels) {
			label->setStyleSheet(VISTMGT("YSS::Editor.StackWidgetTag.Normal"),
				VISTMGT("YSS::Editor.StackWidgetTag.Hover"),
				VISTMGT("YSS::Editor.StackWidgetTag.Pressed"));
		}
	}
	void StackWidgetTagArea::resizeEvent(QResizeEvent* event) {
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
		emit redirectionRequired(filePath, lineNumber, columnNumber);
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
		ContentLabel->setText(VITR("YSS::editor.stackWidgetArea.noFileOpened"));
		Layout->addWidget(ContentLabel, 0, 0, Qt::AlignCenter);
		ContentLabel->setAlignment(Qt::AlignCenter);
	}
}