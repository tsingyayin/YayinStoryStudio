#include "../TextEdit.h"
#include <QDebug>
#include "../LangServer.h"
#include "../../Widgets/ThemeManager.h"
#include "../LangServerManager.h"
#include <QtWidgets/qtextbrowser.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>
#include <QtCore/qfileinfo.h>
#include <QtGui/qsyntaxhighlighter.h>
#include <QtWidgets/qmessagebox.h>

namespace YSSCore::Editor {
	class TextEditPrivate {
		friend class TextEdit;
	protected:
		QTextEdit* Line = nullptr;
		QTextEdit* Text = nullptr;
		QHBoxLayout* Layout = nullptr;
		qint32 LineCount = 0;
		qint8 TabWidth = 4;
		qint32 LastCursorLine = 0;
		QTextCursor LastCursor;
		QFont Font;
		QFontMetricsF* FontMetrics;
		QSyntaxHighlighter* Highlighter;
		bool ReloadTab = true;
		HoverTip* Hover = nullptr;
		TextEditPrivate() {};
		~TextEditPrivate() {
			if (FontMetrics != nullptr) {
				delete FontMetrics;
			}
			// do not delete Highlighter, it will be automaticly deleted by QTextDocument.
		}
	};
	/*!
		\class YSSCore::Editor::TextEdit
		\brief 这是YSS最关键的功能：代码编辑器.
		\since YSSCore 0.13.0
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
		\since YSSCore 0.13.0
		TextEdit的构造函数。
	*/
	TextEdit::TextEdit(QWidget* parent) :YSSCore::Editor::FileEditWidget(parent) {
		d = new TextEditPrivate;
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
		connect(d->Text->document(), &QTextDocument::blockCountChanged, this, &TextEdit::onBlockCountChanged);
		connect(d->Text->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
			d->Line->verticalScrollBar()->setValue(value);
			});
		connect(d->Line->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
			d->Text->verticalScrollBar()->setValue(value);
			});
		connect(d->Text, &QTextEdit::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
		connect(d->Text, &QTextEdit::textChanged, this, &TextEdit::setFileChanged);
	}
	/*!
		\since YSSCore 0.13.0
		TextEdit的析构函数。
	*/
	TextEdit::~TextEdit() {
		delete d;
	}

	void TextEdit::onBlockCountChanged(qint32 count) {
		qint32 delta = count - d->LineCount;
		if (delta > 0) {
			QTextCursor cursor = d->Line->textCursor();
			cursor.movePosition(QTextCursor::End);
			QTextBlockFormat blockFormat = cursor.blockFormat();
			blockFormat.setBackground(YSSTM->getColor("ThemeColor.Editor.Background"));
			blockFormat.setForeground(YSSTM->getColor("ThemeColor.Editor.LineNumber"));
			for (int i = 0; i < delta; i++) {
				d->Line->append(QString::number(d->LineCount + i + 1));
				cursor.movePosition(QTextCursor::Down);
				cursor.setBlockFormat(blockFormat);
			}
		}
		else {
			for (int i = 0; i < -delta; i++) {
				QTextCursor cursor = QTextCursor(d->Line->document()->findBlockByNumber(d->LineCount - i - 1));
				cursor.select(QTextCursor::BlockUnderCursor);
				cursor.removeSelectedText();
				cursor.deleteChar();
			}
		}
		d->LineCount = count;
	}

	bool TextEdit::eventFilter(QObject* obj, QEvent* event) {
		if (obj == d->Text) {
			if (event->type() == QEvent::KeyPress) {
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
				if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
					onTabClicked(keyEvent);
					return true;
				}
				else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
					if (d->Text->textCursor().hasSelection()) { // 不改动选择时回车
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

	void TextEdit::closeEvent(QCloseEvent* event) {
		if (onClose()) {
			event->accept();
		}
		else {
			event->ignore();
		}
	}
	void TextEdit::onTabClicked(QKeyEvent* event) {
		if (event->key() == Qt::Key_Backtab) {
			if (d->Text->textCursor().hasSelection()) {
				QTextCursor cursor = d->Text->textCursor();
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
							for (int i = 0; i < d->TabWidth; i++) {
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
				QTextCursor cursor = d->Text->textCursor();
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
					cursor = d->Text->textCursor();
					cursor.movePosition(QTextCursor::StartOfBlock);
					if (text[0] == '\t') {
						cursor.deleteChar();
					}
					else {
						for (int i = 0; i < d->TabWidth; i++) {
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
			if (d->Text->textCursor().hasSelection()) {
				QTextCursor cursor = d->Text->textCursor();
				//判断cursor的行数量
				int lineCount = cursor.selectedText().count(QChar(0x2029)) + 1;
				bool reverse = cursor.position() == cursor.selectionEnd();
				if (lineCount > 1) {
					for (int i = 0; i < lineCount; i++) {
						cursor.movePosition(QTextCursor::StartOfBlock);
						if (d->ReloadTab) {
							for (int i = 0; i < d->TabWidth; i++) {
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
				QTextCursor cursor = d->Text->textCursor();
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
				cursor = d->Text->textCursor();
				if (allSpace) {
					if (d->ReloadTab) {
						for (int i = 0; i < d->TabWidth; i++) {
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
		QTextCursor cursor = d->Text->textCursor();
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
		cursor = d->Text->textCursor();
		cursor.insertText("\n" + newSpace);
	}

	void TextEdit::onMouseMove(QMouseEvent* event) {
	}

	void TextEdit::onCursorPositionChanged() {
		QTextCursor cursor = d->Text->textCursor();
		int index = cursor.block().blockNumber();
		if (index == d->LastCursorLine) {
			return;
		}
		if (index >= d->Line->document()->blockCount()) {
			onBlockCountChanged(index + 1);
		}
		if (d->Line->document()->findBlockByNumber(index).text().isEmpty()) {
			return;
		}
		int delta = index - d->LastCursorLine;
		QTextBlockFormat format = d->LastCursor.blockFormat();
		format.setBackground(YSSTM->getColor("ThemeColor.Editor.Background"));
		format.setForeground(YSSTM->getColor("ThemeColor.Editor.LineNumber"));
		d->LastCursor.setBlockFormat(format);
		if (delta > 0) {
			d->LastCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, delta);
		}
		else {
			d->LastCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, -delta);
		}
		format.setBackground(YSSTM->getColor("ThemeColor.Editor.Selection"));
		d->LastCursor.setBlockFormat(format);
		d->LastCursorLine = index;
	}

	bool TextEdit::onOpen(const QString& path) {
	
		if (path.isEmpty()) {
			qDebug() << "File path is empty.";
			return false;
		}
		QString ext = QFileInfo(path).suffix();
		YSSCore::Editor::LangServer* server = YSSLSM->routeExt(ext);
		if (server != nullptr) {
			d->Highlighter = server->createHighlighter(d->Text->document());
		}
		else {
			qDebug() << "No highlighter found for extension:" << ext;
		}
		//highlighter 应在 setText之前，否则会触发两次textChanged
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qDebug() << "Failed to open file:" << path;
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
			qDebug() << "Failed to open file:" << path;
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