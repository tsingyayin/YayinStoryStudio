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
		this->setMinimumSize(800, 600);

		Font = QFont("Microsoft YaHei");
		FontMetrics = new QFontMetricsF(Font);

		Line = new QTextBrowser(this);
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
		Text->installEventFilter(this);

		Layout = new QHBoxLayout(this);
		Layout->addWidget(Line);
		Layout->addWidget(Text);
		Layout->setSpacing(0);
		Layout->setContentsMargins(0, 0, 0, 0);

		LastCursor = Line->textCursor();
		LastCursor.movePosition(QTextCursor::Start);
		LastCursorLine = 0;
		connect(Text->document(), &QTextDocument::blockCountChanged, this, &TextEdit::onBlockCountChanged);
		connect(Text->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
			Line->verticalScrollBar()->setValue(value);
			});
		connect(Line->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
			Text->verticalScrollBar()->setValue(value);
			});
		connect(Text, &QTextEdit::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
		connect(Text, &QTextEdit::textChanged, this, &TextEdit::onTextChanged);
	}
	/*!
		\since YSSCore 0.13.0
		TextEdit的析构函数。
	*/
	TextEdit::~TextEdit() {
		if (FontMetrics != nullptr) {
			delete FontMetrics;
		}
		if (Highlighter != nullptr) {
			delete Highlighter;
		}
	}
	/*!
		\since YSSCore 0.13.0
		\a path 要打开的文件路径。
		如果成功打开文件并加载内容，则返回true；否则返回false。
	*/
	bool TextEdit::openFile(const QString& path) {
		FilePath = path;
		if (path.isEmpty()) {
			qDebug() << "File path is empty.";
			return false;
		}
		QString ext = QFileInfo(path).suffix();
		YSSCore::Editor::LangServer* server = YSSLSM->routeExt(ext);
		if (server != nullptr) {
			Highlighter = server->createHighlighter();
			Highlighter->setDocument(Text->document());
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
		Text->setPlainText(in.readAll());
		TextChanged = false;
		LastCursor.movePosition(QTextCursor::Start);
		file.close();
		return true;
	}

	/*!
		\since YSSCore 0.13.0
		\a path 要保存的文件路径。如果为空，则使用当前文件路径。
	*/
	void TextEdit::saveFile(const QString& path) {
		if (path.isEmpty()) {
			if (FilePath.isEmpty()) {
				qDebug() << "File path is empty.";
				return;
			}
		}
		else {
			FilePath = path;
		}
		QFile file(FilePath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			qDebug() << "Failed to open file:" << FilePath;
			return;
		}
		file.write(Text->toPlainText().toUtf8());
		file.close();
	}
	void TextEdit::onBlockCountChanged(qint32 count) {
		qint32 delta = count - LineCount;
		if (delta > 0) {
			QTextCursor cursor = Line->textCursor();
			cursor.movePosition(QTextCursor::End);
			QTextBlockFormat blockFormat = cursor.blockFormat();
			blockFormat.setBackground(YSSTM->getColor("ThemeColor.Editor.Background"));
			blockFormat.setForeground(YSSTM->getColor("ThemeColor.Editor.LineNumber"));
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

	void TextEdit::onCursorPositionChanged() {
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
		format.setBackground(YSSTM->getColor("ThemeColor.Editor.Background"));
		format.setForeground(YSSTM->getColor("ThemeColor.Editor.LineNumber"));
		LastCursor.setBlockFormat(format);
		if (delta > 0) {
			LastCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, delta);
		}
		else {
			LastCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, -delta);
		}
		format.setBackground(YSSTM->getColor("ThemeColor.Editor.Selection"));
		LastCursor.setBlockFormat(format);
		LastCursorLine = index;
	}

	void TextEdit::onTextChanged() {
		TextChanged = true;
	}

	QString TextEdit::getFilePath() const {
		return FilePath;
	}

	QString TextEdit::getFileName() const {
		return QFileInfo(FilePath).fileName();
	}
	bool TextEdit::onClose() {
		if (TextChanged) {
			QMessageBox msgBox;
			QFileInfo fileInfo(FilePath);
			msgBox.setText("Editor.SaveWarning.Message");
			msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Save);
			int ret = msgBox.exec();
			switch (ret) {
			case QMessageBox::Save:
				saveFile();
				TextChanged = false;
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
		if (path.isEmpty()) {
			saveFile(FilePath);
		}
		else {
			saveFile(path);
		}
		TextChanged = false;
		return true;
	}
	bool TextEdit::onSaveAs(const QString& path) {
		saveFile(path);
		TextChanged = false;
		return true;
	}
	bool TextEdit::onReload() {
		if (TextChanged) {
			QMessageBox msgBox;
			msgBox.setText("Editor.SaveWarning.Message");
			msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Save);
			int ret = msgBox.exec();
			switch (ret) {
			case QMessageBox::Save:
				saveFile();
				TextChanged = false;
				break;
			case QMessageBox::Discard:
				break;
			case QMessageBox::Cancel:
				return false;
			default:
				break;
			}
		}
		openFile(FilePath);
		return true;
	}
}