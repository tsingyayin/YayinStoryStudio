#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>
#include <QtCore/qfileinfo.h>
#include <QtGui/qsyntaxhighlighter.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qtextbrowser.h>
#include <Widgets/ThemeManager.h>
#include <General/Log.h>
#include "Editor/private/TextEdit_p.h"
#include "Editor/private/TabCompleterProvider_p.h"
#include "Editor/private/HoverInfoProvider_p.h"
#include "Editor/TextEdit.h"
#include "Editor/LangServer.h"
#include "Editor/LangServerManager.h"
#include "Editor/TabCompleterProvider.h"
#include "Editor/HoverInfoProvider.h"
#include "General/YSSLogger.h"
#include <QtCore/qtimer.h>
#include "Editor/SyntaxHighlighter.h"
#include <General/TranslationHost.h>
#include <QtWidgets/qmessagebox.h>
namespace YSSCore::__Private__ {
	TextEditPrivate::~TextEditPrivate() {
		if (FontMetrics != nullptr) {
			delete FontMetrics;
		}
		/*!
			NOTICE: about some pointers in TextEditPrivate:
			SyntaxHighlighter、TabCompleter、HoverInfoProvider are all
			deleted by TextEdit's QTextDocument.
			TabCompleterWidget and HoverInfoWidget are children of 
			TextEdit's, so they will also be automatically deleted.
		*/
	}

	bool TextEditPrivate::eventFilter(QObject* obj, QEvent* event) {
		if (obj == Text) {
			if (event->type() == QEvent::KeyPress) {
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
				if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
					onTabClicked(keyEvent);
					return true;
				}
				else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
					if (Text->textCursor().hasSelection()) { // 不改动选择时回车
						return false;
					}
					onEnterClicked(keyEvent);
					return true;
				}
				else if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down ||
					keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) {
					if (TabCompleterWidget != nullptr && TabCompleterWidget->isVisible()) {
						if (keyEvent->modifiers() == Qt::NoModifier) {
							onDirectionClicked(keyEvent);
							return true;
						}
					}
					else {
						useKeyboardToMoveCursor = true;
					}
					return false;
				}
				else if (keyEvent->key() == Qt::Key_Escape) {
					onEscapeClicked(keyEvent);
				}
			}
		}
		else if (obj == Text->viewport()) {
			if (event->type() == QEvent::MouseMove) {
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				onMouseMove(mouseEvent);
				return false;
			}
			else if (event->type() == QEvent::Wheel) {
				QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
				return onMouseScroll(wheelEvent);
			}
			else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
				useKeyboardToMoveCursor = false;
				return false;
			}
		}
		else if (obj == q) {
			if (event->type() == QEvent::Close) {
				if (q->onClose()) {
					event->accept();
					return true;
				}
				else {
					event->ignore();
					return false;
				}
			}
			else if (event->type() == QEvent::Hide || event->type() == QEvent::HideToParent) {
				if (HoverInfoWidget != nullptr && HoverInfoWidget->isVisible()) {
					HoverInfoWidget->hide();
				}
				if (TabCompleterWidget != nullptr && TabCompleterWidget->isVisible()) {
					TabCompleterWidget->hide();
				}
				return false;
			}
			else if (event->type() == QEvent::Show || event->type() == QEvent::ShowToParent) {
				if (HoverInfoWidget != nullptr && HoverInfoWidget->isVisible()) {
					HoverInfoWidget->hide();
				}
				if (TabCompleterWidget != nullptr && TabCompleterWidget->isVisible()) {
					TabCompleterWidget->hide();
				}
				return false;
			}
		}
		return false;
	}

	void TextEditPrivate::onBlockCountChanged(qint32 count) {
		qint32 delta = count - LineCount;
		if (delta > 0) {
			QTextCursor cursor = Line->textCursor();
			cursor.movePosition(QTextCursor::End);
			QTextBlockFormat blockFormat = cursor.blockFormat();
			//blockFormat.setBackground(VISTM->getColor("Editor.Background"));
			//blockFormat.setForeground(VISTM->getColor("Editor.LineNumber"));
			for (int i = 0; i < delta; i++) {
				Line->append(QString::number(LineCount + i + 1));
				cursor.movePosition(QTextCursor::Down);
				cursor.setBlockFormat(blockFormat);
			}
		}
		else {
			for (int i = 0; i < -delta; i++) {
				QTextCursor cursor = QTextCursor(Line->document()->findBlockByNumber(LineCount - i - 1));
				cursor.select(QTextCursor::BlockUnderCursor);
				cursor.removeSelectedText();
				cursor.deleteChar();
			}
		}
		LineCount = count;
	}

	static bool isSignSymbol(QChar c) {
		if (0x0020u <= c.unicode() && c.unicode() <= 0x002Fu) {
			return true; // !"#$%&'()*+,-./
		}
		else if (0x003Au <= c.unicode() && c.unicode() <= 0x0040u) {
			return true; // :;<=>?@
		}
		else if (0x005Bu <= c.unicode() && c.unicode() <= 0x0060u) {
			return true; // [\]^_`
		}
		else if (0x007Bu <= c.unicode() && c.unicode() <= 0x007Eu) {
			return true; // {|}~
		}
		else if (c == '\n' || c == '\r' || c == '\t' || c == ' ') {
			return true; // \n, \r, \t, space
		}
		return false;
	}

	void TextEditPrivate::onCursorPositionChanged() {
		if (useKeyboardToMoveCursor) {
			useKeyboardToMoveCursor = false;
			return;
		}
		onCompleter();
		onHoverInfo(false);
		QTextCursor cursor = Text->textCursor();
		int index = cursor.block().blockNumber();
		if (index == LastCursorLine) {
			return;
		}
		if (index >= Line->document()->blockCount()) {
			onBlockCountChanged(index + 1);
		}
		if (Line->document()->findBlockByNumber(index).text().isEmpty()) {
			return;
		}
		int delta = index - LastCursorLine;
		QTextBlockFormat format = LastCursor.blockFormat();
		//format.setBackground(VISTM->getColor("Editor.Background"));
		//format.setForeground(VISTM->getColor("Editor.LineNumber"));
		LastCursor.setBlockFormat(format);
		if (delta > 0) {
			LastCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, delta);
		}
		else {
			LastCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, -delta);
		}
		//format.setBackground(VISTM->getColor("Editor.Selection"));
		LastCursor.setBlockFormat(format);
		LastCursorLine = index;
	}

	void TextEditPrivate::onCompleter() {
		QTextCursor cursor = Text->textCursor();
		if (TabCompleter) {
			QList<YSSCore::Editor::TabCompleterItem> list;
			QString content = cursor.block().text();
			int position = cursor.positionInBlock();
			int RIndex = position; int LIndex = position;
			if (position != 0) {
				for (int i = position - 1; i >= 0; i--) {
					if (isSignSymbol(content[i])) {
						LIndex = i + 1;
						break;
					}
				}
			}
			else {
				LIndex = 0;
			}
			for (int i = position; i < content.length(); i++) {
				if (isSignSymbol(content[i])) {
					RIndex = i;
					break;
				}
			}
			//yDebugF << "LIndex:" << LIndex << "RIndex:" << RIndex << "Position:" << position;
			if (RIndex != position) { // cursor in the middle of a word
				if (TabCompleterWidget->isVisible()) {
					TabCompleterWidget->hide();
				}
				return;
			}
			qint32 line = cursor.block().blockNumber();
			qint32 column = cursor.positionInBlock();
			list = TabCompleter->onTabComplete(line, column, content);
			if (list.isEmpty()) {
				TabCompleterWidget->hide();
				return;
			}
			///yDebugF << "TabCompleterWidget is shown";
			TabCompleterWidget->setCompleterItems(list);
			QRect pos = Text->cursorRect();
			if (not HoverArea) {
				TabCompleterWidget->move(QPoint(pos.x() + 10, pos.y() + 20));
			}
			else {
				TabCompleterWidget->move(Text->mapTo(HoverArea, QPoint(pos.x() + 10, pos.y() + 20)));
			}
			TabCompleterWidget->show();
			Text->setFocus();
		}
	}
	
	void TextEditPrivate::onTabClicked(QKeyEvent* event) {
		if (TabCompleterWidget != nullptr && TabCompleterWidget->isVisible()) {
			onTabClicked_TabCompleter(event);
		}
		else {
			onTabClicked_NormalInput(event);
		}
	}

	void TextEditPrivate::onTabClicked_TabCompleter(QKeyEvent* event) {
		TabCompleterWidget->doComplete();
	}

	void TextEditPrivate::onTabClicked_NormalInput(QKeyEvent* event) {
		if (event->key() == Qt::Key_Backtab) {
			if (Text->textCursor().hasSelection()) {
				QTextCursor cursor = Text->textCursor();
				//判断cursor的行数量
				int lineCount = cursor.selectedText().count(QChar(0x2029)) + 1;
				bool reverse = cursor.position() == cursor.selectionEnd();
				if (lineCount > 1) {
					for (int i = 0; i < lineCount; i++) {
						cursor.movePosition(QTextCursor::EndOfBlock);
						cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
						QString text = cursor.selectedText();
						if (text.length() == 0) {
							continue;
						}
						cursor.movePosition(QTextCursor::StartOfBlock);
						//删除行首1个tab或 TabWidth个空格
						if (text[0] == '\t') {
							cursor.deleteChar();
						}
						else {
							for (int i = 0; i < TabWidth; i++) {
								if (text[i] == ' ') {
									cursor.deleteChar();
								}
								else {
									break;
								}
							}
						}
						if (reverse) {
							cursor.movePosition(QTextCursor::PreviousBlock);
						}
						else {
							cursor.movePosition(QTextCursor::NextBlock);
						}
					}
				}
			}
			else {
				//从行首拉到光标位置
				QTextCursor cursor = Text->textCursor();
				cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
				QString text = cursor.selectedText();
				//判断是否都为空白字符（空格、制表符）
				if (text.length() == 0) {
					return;
				}
				bool allSpace = true;
				for (int i = 0; i < text.length(); i++) {
					if (text[i] != ' ' && text[i] != '\t') {
						allSpace = false;
						break;
					}
				}
				if (allSpace) {
					//删除行首1个tab或 TabWidth个空格
					cursor = Text->textCursor();
					cursor.movePosition(QTextCursor::StartOfBlock);
					if (text[0] == '\t') {
						cursor.deleteChar();
					}
					else {
						for (int i = 0; i < TabWidth; i++) {
							if (text[i] == ' ') {
								cursor.deleteChar();
							}
							else {
								break;
							}
						}
					}
				}
			}
		}
		else {
			if (Text->textCursor().hasSelection()) {
				QTextCursor cursor = Text->textCursor();
				//判断cursor的行数量
				int lineCount = cursor.selectedText().count(QChar(0x2029)) + 1;
				bool reverse = cursor.position() == cursor.selectionEnd();
				if (lineCount > 1) {
					for (int i = 0; i < lineCount; i++) {
						cursor.movePosition(QTextCursor::StartOfBlock);
						if (ReloadTab) {
							for (int i = 0; i < TabWidth; i++) {
								cursor.insertText(QChar(' '));
							}
						}
						else {
							cursor.insertText("\t");
						}
						if (reverse) {
							cursor.movePosition(QTextCursor::PreviousBlock);
						}
						else {
							cursor.movePosition(QTextCursor::NextBlock);
						}
					}
				}
			}
			else {
				QTextCursor cursor = Text->textCursor();
				cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
				QString text = cursor.selectedText();
				//判断是否都为空白字符（空格、制表符）
				bool allSpace = true;
				for (int i = 0; i < text.length(); i++) {
					if (text[i] != ' ' && text[i] != '\t') {
						allSpace = false;
						break;
					}
				}
				cursor = Text->textCursor();
				if (allSpace) {
					if (ReloadTab) {
						for (int i = 0; i < TabWidth; i++) {
							cursor.insertText(QChar(' '));
						}
					}
					else {
						cursor.insertText("\t");
					}
				}
				else {
					cursor.insertText("\t");
				}
			}
		}
	}

	void TextEditPrivate::onEnterClicked(QKeyEvent* event) {
		QTextCursor cursor = Text->textCursor();
		cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
		QString text = cursor.selectedText();
		QString newSpace = "";
		for (int i = 0; i < text.length(); i++) {
			if (text[i] == ' ') {
				newSpace += " ";
			}
			else if (text[i] == '\t') {
				newSpace += "\t";
			}
			else {
				break;
			}
		}
		cursor = Text->textCursor();
		cursor.insertText("\n" + newSpace);
	}

	void TextEditPrivate::onEscapeClicked(QKeyEvent* event) {
		if (TabCompleterWidget != nullptr && TabCompleterWidget->isVisible()) {
			TabCompleterWidget->hide();
		}
		if (HoverInfoWidget != nullptr && HoverInfoWidget->isVisible()) {
			HoverInfoWidget->hide();
		}
	}

	void TextEditPrivate::onDirectionClicked(QKeyEvent* event) {
		if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
			if (TabCompleterWidget != nullptr && TabCompleterWidget->isVisible()) {
				if (event->key() == Qt::Key_Up) {
					TabCompleterWidget->selectPrevious();
				}
				else {
					TabCompleterWidget->selectNext();
				}
			}
		}
	}

	void TextEditPrivate::onMouseMove(QMouseEvent* event) {
		float dist = std::sqrt(std::pow(LastMousePos.x() - event->pos().x(), 2) + std::pow(LastMousePos.y() - event->pos().y(), 2));
		if (dist >  10.0f) { // only reset timer when mouse moved enough distance
			HoverTimer->start(HoverTimeout);
			LastMousePos = event->pos();
		}
		else {
			return;
		}
		HoverTimer->start(HoverTimeout);
		if (HoverInfoWidget!= nullptr && HoverInfoWidget->isVisible()) {
			HoverInfoWidget->hide();
		}
	}

	bool TextEditPrivate::onMouseScroll(QWheelEvent* event) {
		if (HoverInfoWidget && HoverInfoWidget->isVisible()) {
			// scroll the hover info widget
			HoverInfoWidget->scrollBy(-event->angleDelta().y());
			return true;
		}
		else {
			return false;
		}
	}
	
	void TextEditPrivate::onHoverTimeout() {
		onHoverInfo(true);
	}

	void TextEditPrivate::onHoverInfo(bool triggeFromHover) {
		if (HoverInfoProvider != nullptr) {
			QPoint pos = QCursor::pos();
			HoverInfoProvider->d->HoverSetSth = false;
			if (triggeFromHover) {
				if (!Text->viewport()->rect().contains(Text->mapFromGlobal(pos))) {
					HoverTimer->stop();
					HoverInfoWidget->hide();
					return;
				}
			}
			HoverInfoProvider->d->TriggerFromHover = triggeFromHover;
			QTextCursor cursor;
			if (triggeFromHover) {
				cursor = Text->cursorForPosition(Text->mapFromGlobal(pos));
			}
			else {
				cursor = Text->textCursor();
			}
			QString content = cursor.block().text();
			qint32 line = cursor.block().blockNumber();
			qint32 column = cursor.positionInBlock();
			HoverInfoProvider->onMouseHover(line, column, content);
			if (!HoverInfoProvider->d->HoverSetSth) {
				HoverInfoWidget->hide();
			}
			else {
				switch (HoverInfoProvider->d->CurrentFormat) {
					case YSSCore::Editor::HoverInfoProvider::PlainText:
						HoverInfoWidget->setPlainText(HoverInfoProvider->d->Content);
						break;
					case YSSCore::Editor::HoverInfoProvider::Markdown:
						HoverInfoWidget->setMarkdown(HoverInfoProvider->d->Content);
						break;
					case YSSCore::Editor::HoverInfoProvider::Html:
						HoverInfoWidget->setHtml(HoverInfoProvider->d->Content);
						break;
					default:
						HoverInfoWidget->setPlainText(HoverInfoProvider->d->Content);
						break;
				}
				HoverInfoWidget->show();
				if (TabCompleterWidget && TabCompleterWidget->isVisible()) {
					QRect tpos = TabCompleterWidget->geometry();
					HoverInfoWidget->move(tpos.x() + tpos.width(), tpos.y());
				}
				else {
					QRect lpos = Text->cursorRect(cursor);
					if (!HoverArea) {
						HoverInfoWidget->move(QPoint(lpos.x() + 10, lpos.y() + 20));
					}
					else {
						HoverInfoWidget->move(Text->mapTo(HoverArea, QPoint(lpos.x() + 10, lpos.y() + 20)));
					}
				}
			}
			HoverTimer->stop();

		}
	}

	void TextEditPrivate::onScrollBarChanged(int value) {
		QScrollBar* sender = static_cast<QScrollBar*>(this->sender());
		if (sender == Text->verticalScrollBar()) {
			Line->verticalScrollBar()->setValue(value);
		}
		else {
			Text->verticalScrollBar()->setValue(value);
		}
		if (TabCompleterWidget != nullptr && TabCompleterWidget->isVisible()) {
			QRect pos = Text->cursorRect();
			TabCompleterWidget->move(QPoint(pos.x() + 10, pos.y() + 20));
		}
	}
}
namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::TextEdit
		\brief 这是YSS最关键的功能：代码编辑器.
		\since Visindigo 0.13.0
		\inmodule YSSCore

		TextEdit是Yayin Story Studio中最关键、最核心的功能，即代码编辑器。

		此类相比于Qt提供的QTextEdit，额外提供了其所缺失的现代代码编辑必备的几项基本功能：
		\list 1
			\li 自动缩进
			\li 多行Tab的缩进和反缩进
			\li 行号
		\endlist
		此外，TextEdit同样像QTextEdit一样，支持Qt的所有文本编辑功能，如撤销、重做、查找替换等。
		并且支持使用QSyntaxHighlighter高亮语法。考虑到代码文件间可能存在符号关联关系，YSS扩展了
		语法高亮的概念，除了最基本的着色操作仍需要使用QSyntaxHighlighter外，YSS提供了LangServer
		类来处理更复杂的语法高亮需求，如代码补全、错误提示等。

		要在YSS中使用最基本的语法着色，请实现自己的LangServer类，并将其中的createHighlighter()
		方法返回一个QSyntaxHighlighter实例。之后再将您的LangServer类注册到LangServerManager中。

		\note 一般来说，TextEdit不会单独使用。若需要使用此类，我们强烈建议将其作为您的组件的一部分，
		而不是继承并扩展此类。
	*/
	/*!
		\since Visindigo 0.13.0
		TextEdit的构造函数。
	*/
	TextEdit::TextEdit(QWidget* parent) :YSSCore::Editor::FileEditWidget(parent) {
		d = new YSSCore::__Private__::TextEditPrivate;
		d->q = this;
		//this->setMinimumSize(800, 600);
		this->setMouseTracking(true);
		d->Font = qApp->font();
		d->FontMetrics = new QFontMetricsF(d->Font);

		d->Line = new QTextBrowser(this);
		d->Line->setReadOnly(true);
		d->Line->setMaximumWidth(100);
		d->Line->setAlignment(Qt::AlignRight);
		d->Line->document()->setDefaultFont(QFont("Microsoft YaHei"));
		d->Line->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->Line->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		d->Text = new QTextEdit(this);
		d->Text->document()->setDefaultFont(QFont("Microsoft YaHei"));
		double rawSpaceWidth = d->FontMetrics->size(Qt::TextSingleLine, " ").width();
		double tabStopDistance = d->TabWidth * rawSpaceWidth * this->devicePixelRatioF();
		d->Text->setTabStopDistance(qMax(20.0, tabStopDistance));
		//vgDebug << "TabStopDistance:" << d->Text->tabStopDistance() << d->TabWidth * d->FontMetrics->size(Qt::TextSingleLine, " ").width();
		d->Text->setLineWrapMode(QTextEdit::NoWrap);
		d->Text->installEventFilter(d);
		d->Text->viewport()->setMouseTracking(true);
		d->Text->viewport()->installEventFilter(d);
		d->Text->verticalScrollBar()->setStyleSheet(VISTMGT("YSS::NormalScrollBar", this));
		this->installEventFilter(d);
		d->Layout = new QHBoxLayout(this);
		d->Layout->addWidget(d->Line);
		d->Layout->addWidget(d->Text);
		d->Layout->setSpacing(0);
		d->Layout->setContentsMargins(0, 0, 0, 0);

		d->LastCursor = d->Line->textCursor();
		d->LastCursor.movePosition(QTextCursor::Start);
		d->LastCursorLine = 0;

		d->HoverTimer = new QTimer(this);
		d->HoverTimer->setInterval(d->HoverTimeout);
		connect(d->Text->document(), &QTextDocument::blockCountChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onBlockCountChanged);
		connect(d->Text->verticalScrollBar(), &QScrollBar::valueChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onScrollBarChanged);
		connect(d->Line->verticalScrollBar(), &QScrollBar::valueChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onScrollBarChanged);
		connect(d->Text, &QTextEdit::cursorPositionChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onCursorPositionChanged);
		connect(d->Text, &QTextEdit::textChanged, this, &TextEdit::setFileChanged);
		connect(d->HoverTimer, &QTimer::timeout, this->d, &YSSCore::__Private__::TextEditPrivate::onHoverTimeout);
	}
	/*!
		\since Visindigo 0.13.0
		TextEdit的析构函数。
	*/
	TextEdit::~TextEdit() {
		// d->Text must be delete first.
		// If we do nothing with this object, d->Text will be automaticly deleted by
		// Qt's parent-child system. However, due to our YSSCore::Editor::SyntaxHighlighter
		// object is initialized with this object, but ownership belongs to QTextDocument in d->Text.
		// IF SyntaxHighlighter wants to access some of this object's members in its destructor, 
		// it will cause a crash as our d pointer is already deleted.
		// So, Delete d->Text first to make sure SyntaxHighlighter is deleted before we delete d.
		if (d->HoverInfoWidget) {
			d->HoverInfoWidget->setParent(d->Text);
		}
		if (d->TabCompleterWidget) {
			d->TabCompleterWidget->setParent(d->Text);
		}
		delete d->Text;
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置鼠标悬停提示的显示区域。如果不设置或设置为nullptr，那么悬停区域
		永远不会超出TextEdit的范围。这可能在很多情况下会导致内容被遮挡，
		因此我们建议您将悬停区域设置为一个更大的区域，如您的组件的主窗口等。
	*/
	void TextEdit::setHoverArea(QWidget* area) {
		if (not area) {
			d->HoverArea = nullptr;
			if (d->HoverInfoWidget){
				d->HoverInfoWidget->setParent(d->Text);
			}
			if (d->TabCompleterWidget){
				d->TabCompleterWidget->setParent(d->Text);
			}
		}
		d->HoverArea = area;
		if (d->HoverInfoWidget){
			d->HoverInfoWidget->setParent(area);
		}
		if (d->TabCompleterWidget){
			d->TabCompleterWidget->setParent(area);
		}
	}
	/*!
		\since Visindigo 0.13.0
		设置TextEdit中的文本内容。此函数会替换掉TextEdit中原有的全部文本内容。
	*/
	void TextEdit::setPlainText(const QString& text) {
		d->Text->setPlainText(text);
	}

	/*!
		\since Visindigo 0.13.0
		获取TextEdit中的文本内容。此函数会返回TextEdit中全部的文本内容。
	*/
	QString TextEdit::getPlainText() const {
		return d->Text->toPlainText();
	}

	/*!
		\since Visindigo 0.13.0
		将光标移动到指定行。行号从1开始。
	*/
	void TextEdit::moveCursorToLine(int lineNumber) {
		if (lineNumber < 1 || lineNumber > d->Text->document()->blockCount()) {
			return;
		}
		QTextCursor cursor = d->Text->textCursor();
		cursor.movePosition(QTextCursor::Start);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
		d->Text->setTextCursor(cursor);
	}

	/*!
		\since Visindigo 0.13.0
		获取当前光标所在的行号。行号从1开始。
	*/
	int TextEdit::getCurrentLineNumber() const {
		QTextCursor cursor = d->Text->textCursor();
		return cursor.block().blockNumber() + 1;
	}

	/*!
		\since Visindigo 0.13.0
		设置鼠标悬停提示的超时时间。单位为毫秒。
	*/
	void TextEdit::setHoverTimeout(qint32 ms) {
		d->HoverTimeout = ms;
		d->HoverTimer->setInterval(ms);
	}

	/*!
		\since Visindigo 0.13.0
		获取鼠标悬停提示的超时时间。单位为毫秒。
	*/
	qint32 TextEdit::getHoverTimeout() const {
		return d->HoverTimeout;
	}

	/*!
		\since Visindigo 0.13.0
		设置Tab键是否使用空格进行缩进。默认为false，即使用制表符进行缩进。
	*/
	void TextEdit::setTabReload(bool reload) {
		d->ReloadTab = reload;
	}

	/*!
		\since Visindigo 0.13.0
		获取Tab键是否使用空格进行缩进。
	*/
	bool TextEdit::isTabReload() const {
		return d->ReloadTab;
	}

	/*!
		\since Visindigo 0.13.0
		设置Tab键的缩进宽度。单位为字符数。默认为4。
	*/
	QTextDocument* TextEdit::getDocument() const {
		return d->Text->document();
	}

	bool TextEdit::eventFilter(QObject* obj, QEvent* event) {
		return false;
	}

	void TextEdit::showEvent(QShowEvent* event) {

	}

	void TextEdit::closeEvent(QCloseEvent* event) {

	}

	/*!
		\since Visindigo 0.13.0
		按给定行号和列重定位光标。行号和列号均从0开始。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用cursorToPosition()函数。
	*/
	bool TextEdit::onCursorToPosition(int line, int column) {
		vgDebug << "Cursor to position: line" << line << "column" << column;
		if (line < 0 || line >= d->Text->document()->blockCount()) {
			return false;
		}
		QTextBlock block = d->Text->document()->findBlockByNumber(line);
		if (column < 0 || column > block.length()) {
			return false;
		}
		QTextCursor cursor(block);
		cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
		d->Text->setTextCursor(cursor);
		return true;
	}
	/*!
		\since Visindigo 0.13.0
		打开一个文件。只有文件完全打开成功才会返回true，其他任何失败情况均返回false。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用openFile()函数。
	*/
	bool TextEdit::onOpen(const QString& path) {
		QString ext = QFileInfo(path).suffix();
		YSSCore::Editor::LangServer* server = YSSLSM->routeExt(ext);
		if (server != nullptr) {
			if (d->Highlighter != nullptr) {
				delete d->Highlighter;
			}
			d->Highlighter = server->createHighlighter(this);
			if (d->TabCompleter != nullptr) {
				delete d->TabCompleter;
			}
			d->TabCompleter = server->createTabCompleter(this);
			if (d->TabCompleterWidget != nullptr) {
				delete d->TabCompleterWidget;
			}
			if (d->TabCompleter != nullptr) {
				d->TabCompleterWidget = new YSSCore::__Private__::TabCompleterWidget(d->Text);
			}
			d->HoverInfoProvider = server->createHoverInfoProvider(this);
			if (d->HoverInfoWidget != nullptr) {
				delete d->HoverInfoWidget;
			}
			if (d->HoverInfoProvider != nullptr) {
				d->HoverInfoWidget = new YSSCore::__Private__::HoverInfoWidget(d->Text);
			}
		}
		else {
			yInfoF << "No Language server found for extension:" << ext;
		}
		//highlighter 应在 setText之前，否则会触发两次textChanged
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			vgErrorF << "Failed to open file:" << path;
			return false;
		}
		QTextStream in(&file);
		in.setEncoding(QStringConverter::Utf8);
		d->Text->setPlainText(in.readAll());
		cancelFileChanged();
		d->LastCursor.movePosition(QTextCursor::Start);
		file.close();
		return true;
	}

	/*!
		\since Visindigo 0.13.0
		关闭当前文件。只有当文件成功关闭时才会返回true，其他任何失败情况均返回false。
	*/
	bool TextEdit::onClose() {
		if (isFileChanged()) {
			int ret = QMessageBox::warning(this, VITR("YSS::editor.saveWarning.title"), 
				VITR("YSS::editor.saveWarning.message").arg(getFileName()),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
				QMessageBox::Save);
			switch (ret) {
			case QMessageBox::Save:
				saveFile();
				return true;
			case QMessageBox::Discard:
				// Don't save was clicked
				return true;
			case QMessageBox::Cancel:
				// Cancel was clicked
				return false;
			default:
				// should never be reached
				return false;
			}
		}
		else {
			return true;
		}
	}

	/*!
		\since Visindigo 0.13.0
		保存当前文件。只有当文件成功保存时才会返回true，其他任何失败情况均返回false。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用saveFile()函数。
	*/
	bool TextEdit::onSave(const QString& path) {
		QFile file(path);
		QFileInfo fileInfo(path);
		if (not fileInfo.isWritable()) {
			QMessageBox::warning(this, 
				VITR("YSS::editor.textEdit.readOnly.title"), VITR("YSS::editor.textEdit.readOnly.message").arg(fileInfo.fileName()),
				QMessageBox::Ok);
			vgWarning << "File is read-only:" << path << ",";
			return false;
		}
		else {
			if (not file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				vgErrorF << "Failed to save file:" << path;
				return false;
			}
			file.write(d->Text->toPlainText().toUtf8());
			file.close();
			emit fileSaved(path);
			return true;
		}
	}

	/*!
		\since Visindigo 0.13.0
		重新加载当前文件。只有当文件成功重新加载时才会返回true，其他任何失败情况均返回false。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用reloadFile()函数。
	*/
	bool TextEdit::onReload() {
		if (isFileChanged()) {
			QMessageBox msgBox;
			msgBox.setText("Editor.SaveWarning.Message");
			msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Save);
			int ret = msgBox.exec();
			switch (ret) {
			case QMessageBox::Save:
				saveFile();
				break;
			case QMessageBox::Discard:
				break;
			case QMessageBox::Cancel:
				return false;
			default:
				break;
			}
		}
		openFile(getFilePath());
		return true;
	}


	bool TextEdit::onCopy() {
		d->Text->copy();
		return true;
	}

	bool TextEdit::onCut() {
		d->Text->cut();
		return true;
	}

	bool TextEdit::onPaste() {
		d->Text->paste();
		return true;
	}

	bool TextEdit::onUndo() {
		d->Text->undo();
		return true;
	}

	bool TextEdit::onRedo() {
		d->Text->redo();
		return true;
	}

	bool TextEdit::onSelectAll() {
		d->Text->selectAll();
		return true;
	}
}