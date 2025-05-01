#include "../TextEdit.h"
#include "../AStorySyntaxHighlighter.h"
#include <QDebug>
namespace YSE::Editor {
	TextEdit::TextEdit(QWidget* parent):QWidget(parent) {
		this->setMinimumSize(800, 600);

		Font = QFont("Microsoft YaHei");
		FontMetrics = new QFontMetricsF(Font);

		Line = new QTextEdit(this);
		Line->setReadOnly(true);
		Line->setMaximumWidth(100);
		Line->setAlignment(Qt::AlignRight);
		Line->document()->setDefaultFont(QFont("Microsoft YaHei"));
		Line->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		Line->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		Text = new QTextEdit(this);
		Text->document()->setDefaultFont(QFont("Microsoft YaHei"));
		Text->setTabStopDistance(qMax(20.0, TabWidth * FontMetrics->size(Qt::TextSingleLine, " ").width()));
		Text->setLineWrapMode(QTextEdit::NoWrap);

		Layout = new QHBoxLayout(this);
		Layout->addWidget(Line);
		Layout->addWidget(Text);

		connect(Text->document(), &QTextDocument::blockCountChanged, this, &TextEdit::onBlockCountChanged);
		connect(Text->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
			Line->verticalScrollBar()->setValue(value);
			});
		connect(Line->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
			Text->verticalScrollBar()->setValue(value);
			});
		Text->installEventFilter(this);

		Highlighter = new AStorySyntaxHighlighter(Text->document());
	}

	void TextEdit::onBlockCountChanged(qint32 count) {
		qint32 delta = count - LineCount;
		if (delta > 0) {
			for (int i = 0; i < delta; i++) {
				Line->append(QString::number(LineCount+i+1));
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

	bool TextEdit::eventFilter(QObject* obj, QEvent* event) {
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
			}
			else if (event->type() == QEvent::MouseMove) {
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				onMouseMove(mouseEvent);
				return false;
			}
		}
		return false;
	}

	void TextEdit::onTabClicked(QKeyEvent* event) {
		if (event->key() == Qt::Key_Backtab) {
			if (Text->textCursor().hasSelection()) {
				QTextCursor cursor = Text->textCursor();
				//判断cursor的行数量
				int lineCount = cursor.selectedText().count(QChar(0x2029)) + 1;
				qDebug() << lineCount;
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
						cursor.movePosition(QTextCursor::NextBlock);
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
				qDebug() << lineCount;
				if (lineCount > 1) {
					//为首行添加缩进
					cursor.movePosition(QTextCursor::StartOfBlock);
					if (ReloadTab) {
						for (int i = 0; i < TabWidth; i++) {
							cursor.insertText(QChar(' '));
						}
					}
					else {
						cursor.insertText("\t");
					}
					//为后续行添加缩进
					for (int i = 1; i < lineCount; i++) {
						cursor.movePosition(QTextCursor::NextBlock);
						cursor.movePosition(QTextCursor::StartOfBlock);
						if (ReloadTab) {
							for (int i = 0; i < TabWidth; i++) {
								cursor.insertText(QChar(' '));
							}
						}
						else {
							cursor.insertText("\t");
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
					qDebug() << "insert tab";
					cursor.insertText("\t");
				}
			}
		}
	}

	void TextEdit::onEnterClicked(QKeyEvent* event) {
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

	void TextEdit::onMouseMove(QMouseEvent* event) {
		
	}
}