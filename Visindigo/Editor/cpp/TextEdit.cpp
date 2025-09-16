/*
*	Yayin Story Studio - Core Library
* 
*/
#include "../../Widgets/ThemeManager.h"
#include "../../General/Log.h"
#include "../TextEdit.h"
#include "../LangServer.h"
#include "../LangServerManager.h"
#include "../TabCompleterProvider.h"
#include "../private/TextEdit_p.h"
#include "../private/TabCompleterProvider_p.h"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>
#include <QtCore/qfileinfo.h>
#include <QtGui/qsyntaxhighlighter.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qregularexpression.h>

namespace Visindigo::__Private__ {

	TextEditPrivate::~TextEditPrivate() {
		if (FontMetrics != nullptr) {
			delete FontMetrics;
		}
		if (TabCompleterWidget != nullptr) {
			delete TabCompleterWidget;
		}
		// do not delete Highlighter, it will be automaticly deleted by QTextDocument.
	}

	void TextEditPrivate::onBlockCountChanged(qint32 count) {
		qint32 delta = count - LineCount;
		if (delta > 0) {
			QTextCursor cursor = Line->textCursor();
			cursor.movePosition(QTextCursor::End);
			QTextBlockFormat blockFormat = cursor.blockFormat();
			blockFormat.setBackground(YSSTM->getColor("Editor.Background"));
			blockFormat.setForeground(YSSTM->getColor("Editor.LineNumber"));
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
		}else if (0x003Au <= c.unicode() && c.unicode() <= 0x0040u) {
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
		onCompleter();
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
		format.setBackground(YSSTM->getColor("Editor.Background"));
		format.setForeground(YSSTM->getColor("Editor.LineNumber"));
		LastCursor.setBlockFormat(format);
		if (delta > 0) {
			LastCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, delta);
		}
		else {
			LastCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, -delta);
		}
		format.setBackground(YSSTM->getColor("Editor.Selection"));
		LastCursor.setBlockFormat(format);
		LastCursorLine = index;
	}

	void TextEditPrivate::onCompleter() {
		QTextCursor cursor = Text->textCursor();
		if (TabCompleter) {
			QList<Visindigo::Editor::TabCompleterItem> list;
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
			yDebugF << "LIndex:" << LIndex << "RIndex:" << RIndex << "Position:" << position;
			if (RIndex != position) { // cursor in the middle of a word
				if (TabCompleterWidget->isVisible()) {
					TabCompleterWidget->hide();
				}
				return;
			}
			QString wordContent = content.mid(LIndex, RIndex - LIndex);
			yDebugF << "WordContent:" << wordContent;
			/*if (wordContent.isEmpty()) {
				TabCompleterWidget->hide();
				return;
			}*/
			list = TabCompleter->onTabComplete(position, content, wordContent);
			if (list.isEmpty()) {
				TabCompleterWidget->hide();
				return;
			}
			yDebugF << "TabCompleterWidget is shown";
			TabCompleterWidget->setCompleterItems(list);
			QRect pos = Text->cursorRect();
			TabCompleterWidget->move(QPoint(pos.x() + 10, pos.y() + 20));
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

	void TextEditPrivate::onTabClicked_NormalInput(QKeyEvent * event) {
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
namespace Visindigo::Editor {
	/*!
		\class Visindigo::Editor::TextEdit
		\brief 这是YSS最关键的功能：代码编辑器.
		\since Visindigo 0.13.0
		\inmodule Visindigo

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
	TextEdit::TextEdit(QWidget* parent) :Visindigo::Editor::FileEditWidget(parent) {
		d = new Visindigo::__Private__::TextEditPrivate;
		this->setMinimumSize(800, 600);

		d->Font = QFont("Microsoft YaHei");
		d->FontMetrics = new QFontMetricsF(d->Font);

		d->Line = new QTextBrowser(this);
		d->Line->setReadOnly(true);
		d->Line->setMaximumWidth(100);
		d->Line->setAlignment(Qt::AlignRight);
		d->Line->document()->setDefaultFont(QFont("Microsoft YaHei"));
		d->Line->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->Line->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

		d->Text = new QTextEdit(this);
		d->Text->document()->setDefaultFont(QFont("Microsoft YaHei"));
		d->Text->setTabStopDistance(qMax(20.0, d->TabWidth * d->FontMetrics->size(Qt::TextSingleLine, " ").width()));
		d->Text->setLineWrapMode(QTextEdit::NoWrap);
		d->Text->installEventFilter(this);

		d->Layout = new QHBoxLayout(this);
		d->Layout->addWidget(d->Line);
		d->Layout->addWidget(d->Text);
		d->Layout->setSpacing(0);
		d->Layout->setContentsMargins(0, 0, 0, 0);

		d->LastCursor = d->Line->textCursor();
		d->LastCursor.movePosition(QTextCursor::Start);
		d->LastCursorLine = 0;
		connect(d->Text->document(), &QTextDocument::blockCountChanged, this->d, &Visindigo::__Private__::TextEditPrivate::onBlockCountChanged);
		connect(d->Text->verticalScrollBar(), &QScrollBar::valueChanged, this->d, &Visindigo::__Private__::TextEditPrivate::onScrollBarChanged); 
		connect(d->Line->verticalScrollBar(), &QScrollBar::valueChanged, this->d, &Visindigo::__Private__::TextEditPrivate::onScrollBarChanged);
		connect(d->Text, &QTextEdit::cursorPositionChanged, this->d, &Visindigo::__Private__::TextEditPrivate::onCursorPositionChanged);
		connect(d->Text, &QTextEdit::textChanged, this, &TextEdit::setFileChanged);
	}
	/*!
		\since Visindigo 0.13.0
		TextEdit的析构函数。
	*/
	TextEdit::~TextEdit() {
		delete d;
	}

	bool TextEdit::eventFilter(QObject* obj, QEvent* event) {
		if (obj == d->Text) {
			if (event->type() == QEvent::KeyPress) {
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
				if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
					d->onTabClicked(keyEvent);
					return true;
				}
				else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
					if (d->Text->textCursor().hasSelection()) { // 不改动选择时回车
						return false;
					}
					d->onEnterClicked(keyEvent);
					return true;
				}
				else if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
					if (d->TabCompleterWidget!= nullptr && d->TabCompleterWidget->isVisible()) {
						if (keyEvent->modifiers() == Qt::NoModifier) {
							d->onDirectionClicked(keyEvent);
							return true;
						}
					}
					return false;
				}
			}
			else if (event->type() == QEvent::MouseMove) {
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				d->onMouseMove(mouseEvent);
				return false;
			}
		}
		return false;
	}

	void TextEdit::closeEvent(QCloseEvent* event) {
		if (onClose()) {
			event->accept();
		}
		else {
			event->ignore();
		}
	}

	bool TextEdit::onOpen(const QString& path) {
		if (path.isEmpty()) {
			yWarningF << "File path is empty.";
			return false;
		}
		QString ext = QFileInfo(path).suffix();
		Visindigo::Editor::LangServer* server = YSSLSM->routeExt(ext);
		if (server != nullptr) {
			if (d->Highlighter != nullptr) {
				delete d->Highlighter;
			}
			d->Highlighter = server->createHighlighter(d->Text->document());
			if (d->TabCompleter != nullptr) {
				delete d->TabCompleter;
			}
			d->TabCompleter = server->createTabCompleter(d->Text->document());
			if (d->TabCompleterWidget != nullptr) {
				delete d->TabCompleterWidget;
			}
			if (d->TabCompleter != nullptr) {
				d->TabCompleterWidget = new Visindigo::__Private__::TabCompleterWidget(d->Text, d->Text);
			}
		}
		else {
			yInfoF << "No Language server found for extension:" << ext;
		}
		//highlighter 应在 setText之前，否则会触发两次textChanged
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			yErrorF << "Failed to open file:" << path;
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

	bool TextEdit::onClose() {
		if (isFileChanged()) {
			QMessageBox msgBox;
			QFileInfo fileInfo(getFilePath());
			msgBox.setText("Editor.SaveWarning.Message");
			msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Save);
			int ret = msgBox.exec();
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

	bool TextEdit::onSave(const QString& path) {
		QFile file(path);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			yErrorF << "Failed to save file:" << path;
			return false;
		}
		file.write(d->Text->toPlainText().toUtf8());
		file.close();
		return true;
	}
	
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
}