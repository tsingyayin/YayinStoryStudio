#include "Widgets/Terminal.h"
#include "Widgets/private/Terminal_p.h"
#include <QtWidgets/qtextbrowser.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtGui/qtextobject.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qiodevice.h>
#include <QtGui/qevent.h>
#include "General/Log.h"
#include "Utility/Console.h"
#include "General/CommandHost.h"
#include "General/TranslationHost.h"
/*
单一控制字符的控制序列格式如下：
M: 上移一行
7: 保存光标位置
8: 恢复光标位置
H: 插入一个水平制表符
ANSI \033[ 开头的控制序列格式如下：
[<p1>;<p2>...<command>
其中<p1>, <p2>等是参数，<command>是控制命令
目前支持的控制命令包括：
A: 光标上移, p1 行数
B: 光标下移, p1 行数
C: 光标右移, p1 列数
D: 光标左移, p1 列数
E: 光标下移并回到行首, p1 行数
F: 光标上移并回到行首, p1 行数
G: 移动光标到指定列, p1 列数
H: 移动光标到指定位置, p1 行数, p2 列数
l: 将光标移到下一个制表符位置（同一行）,p1 移动制表符列数
J: 清除屏幕, p1 参数为0清除光标位置到行末, 1清除行首到光标位置, 2清除整行
K: 清除行, p1 参数为0清除光标位置到行末, 1清除行首到光标位置, 2清除整行
L: 在光标所在位置插入行, p1 行数
M: 从光标所在位置开始删除行, p1 行数
P: 删除光标所在位置及之后的字符, p1 个
S: 向上滚动文字, p1 行数
T: 向下滚动文字, p1 行数
X: 从光标所在位置用空格替换字符, p1 个
Z: 将光标移到上一个制表符位置（同一行）,p1 移动制表符列数

c: 报告终端类型，回报格式为\033[?1;2c
d: 移动光标到指定行, p1 行数
f: 同H
(0)g: 删除当前制表符列中的制表符
(3)g: 清除本行所有制表符
(?12)h: 启用光标闪烁
(?12)l: 停用光标闪烁
(?25)h: 显示光标
(?25)l: 隐藏光标
(?1)h: 光标应用模式
(?1)l: 光标数字模式（回报光标键的数字而非控制光标位置）
m: 颜色和格式，具体细节在Visindigo::Utility::Console中有所体现。
n: 报告光标位置，回报格式为\033[行数;列数
(num 0x20)q: 设置光标样式，num为0-6
r: 设置滚动区域, p1 行数, p2 行数
s: 保存光标位置
u: 恢复光标位置

@: 在光标位置插入空格, p1 个
=: 小键盘应用模式
>: 小键盘数字模式
*/
namespace Visindigo::__Private__ {
	void TerminalPrivate::onANSILineReceived(const QString& line) {
		QString debugLine = line;
		//qDebug() << debugLine.replace("\r", "\\r").replace("\n", "\\n");
		qint32 normalTextStart = 0;
		for (int i = 0; i < line.length(); ++i) {
			auto chr = line.at(i);
			if (chr == '\033') {
				if (normalTextStart < i) {
					insertPlainText(line.mid(normalTextStart, i - normalTextStart));
				}
				auto rBegin = line.at(i + 1 > line.size() ? line.size() - 1 : i + 1);
				if (rBegin != '[') { // simple control sequence, start with \033 and followed by a single character
					switch (rBegin.toLatin1()) {
					case 'M':
						{
							auto cursor = consoleView->textCursor();
							cursor.movePosition(QTextCursor::Up);
							consoleView->setTextCursor(cursor);
							break;
						}
					case 'H':
						consoleView->textCursor().insertText("\t");
						break;
					case '7':
						savedCursorLine = consoleView->textCursor().blockNumber() - getFirstLineInViewport();
						savedCursorCol = consoleView->textCursor().positionInBlock();
						break;
					case '8':
						if (savedCursorLine != -1 && savedCursorCol != -1) {
							auto cursor = consoleView->textCursor();
							qint32 targetLine = getFirstLineInViewport() + savedCursorLine;
							qint32 delta = targetLine - cursor.blockNumber();
							cursor.movePosition(QTextCursor::StartOfLine);
							cursor.movePosition(delta > 0 ? QTextCursor::Down : QTextCursor::Up, QTextCursor::MoveAnchor, std::abs(delta));
							cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, savedCursorCol);
						}
						break;
					}
					i += 1;
					normalTextStart = i + 1;
				}
				else { // complex control sequence start with \033[
					int j = i + 2;
					for (; j < line.length(); ++j) {
						if (line[j] >= '@' && line[j] <= 'Z' ||
							line[j] >= 'a' && line[j] <= 'z' ||
							line[j] == '=' || line[j] == '>') {
							break;
						}
					}
					if (j < line.length()) {
						QChar command = line.at(j);
						QString content = line.mid(i + 2, j - (i + 2)); // may be empty, depending on the command
						onComplexControlDetected(command, content);
						i = j;
						normalTextStart = j + 1;
					} else {
						i = j - 1;
						normalTextStart = j;
					}
				}
			}
			else if (chr == '\n') {
					// 在当前块之后新增一个块。不能假定光标在行的任何位置，也不能假定光标在任何行
				if (normalTextStart < i) {
					insertPlainText(line.mid(normalTextStart, i - normalTextStart));
				}		
				normalTextStart = i + 1;
				auto cursor = consoleView->textCursor();
				auto column = cursor.positionInBlock();
				cursor.movePosition(QTextCursor::EndOfBlock);
				//如果下一行不存在，则创建一个新行
				if (not cursor.movePosition(QTextCursor::Down)) {
					cursor.insertBlock();
				}
				cursor.movePosition(QTextCursor::StartOfLine);
				
				auto contentOfNextLine = cursor.block().text();
				if (column > contentOfNextLine.length()) {
					cursor.movePosition(QTextCursor::EndOfLine);
					cursor.insertText(QString(column - contentOfNextLine.length(), ' '));
				}
				cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
				if (autoScroll) {
					consoleView->verticalScrollBar()->setValue(cursor.blockNumber());
				}
				consoleView->setTextCursor(cursor);
				consoleView->setCurrentCharFormat(QTextCharFormat());
			}
			else if (chr == '\r') {
				if (normalTextStart < i) {
					insertPlainText(line.mid(normalTextStart, i - normalTextStart));
				}
				normalTextStart = i + 1;
				auto cursor = consoleView->textCursor();
				cursor.movePosition(QTextCursor::StartOfLine);
				consoleView->setTextCursor(cursor);
			}
		}
		if (normalTextStart < line.length()) {
			insertPlainText(line.mid(normalTextStart));
		}
	}
	void TerminalPrivate::insertPlainText(const QString& text) {
		if (text.isEmpty()) {
			return;
		}
		auto cursor = consoleView->textCursor();
		if (cursor.atBlockEnd()) {
			cursor.insertText(text);
		}
		else {
			cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, text.size());
			cursor.insertText(text);
		}
		cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, text.size());
		consoleView->setTextCursor(cursor);
	}
	qint32 TerminalPrivate::getFirstLineInViewport() const {
		auto cursor = consoleView->cursorForPosition(consoleView->viewport()->rect().topLeft() + QPoint(2, 2));
		return cursor.blockNumber();
	}

	qint32 TerminalPrivate::getLastLineInViewport() const {
		auto cursor = consoleView->cursorForPosition(consoleView->viewport()->rect().bottomLeft() - QPoint(2, 2));
		return cursor.blockNumber();
	}

	void TerminalPrivate::onComplexControlDetected(const QChar& command, const QString& content) {
		switch (command.toLatin1()) {
		case 'A': // 光标上移, p1 行数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, content.toInt());
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'B': // 光标下移, p1 行数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, content.toInt());
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'C': // 光标右移, p1 列数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, content.toInt());
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'D': // 光标左移, p1 列数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, content.toInt());
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'E': // 光标下移并回到行首, p1 行数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, content.toInt());
			cursor.movePosition(QTextCursor::StartOfLine);
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'F': // 光标上移并回到行首, p1 行数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, content.toInt());
			cursor.movePosition(QTextCursor::StartOfLine);
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'G': // 移动光标到指定列, p1 列数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::StartOfLine);
			cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, content.toInt());
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'f':
		case 'H': // 移动光标到指定位置, p1 行数, p2 列数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Start);
			auto params = content.split(';');
			if (params.size() >= 1) {
				qint32 lineNum = params[0].toInt();
				qint32 viewportFirstLine = getFirstLineInViewport();
				qint32 targetLine = viewportFirstLine + lineNum;
				qint32 currentLine = cursor.blockNumber();
				qint32 moveDelta = targetLine - currentLine;
				cursor.movePosition(moveDelta > 0 ? QTextCursor::Down : QTextCursor::Up, QTextCursor::MoveAnchor, std::abs(moveDelta));
			}
			if (params.size() >= 2) {
				cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, params[1].toInt());
			}
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'I': // 将光标移到下一个制表符位置（同一行）,p1 移动制表符列数
		case 'Z': // 将光标移到上一个制表符位置（同一行）,p1 移动制表符列数
		case 'g': // (0)g: 删除当前制表符列中的制表符, (3)g: 清除本行所有制表符
		{
			// 由于需要为每一行维护制表符位置，这个几个命令比较麻烦，统一暂不支持
			break;
		}
		case 'J': // 清除屏幕, p1 参数为0清除光标位置到行末, 1清除行首到光标位置, 2清除整行
		{
			auto cursor = consoleView->textCursor();
			switch (content.toInt()) {
			case 0:
			{
				cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
				QString content = cursor.selectedText();
				// 用空格替换选中的文本
				cursor.insertText(QString(content.length(), ' '));
				qint32 lastLine = getLastLineInViewport();
				qint32 currentLine = cursor.blockNumber();
				for (qint32 line = currentLine + 1; line <= lastLine; ++line) {
					cursor.movePosition(QTextCursor::Down);
					cursor.movePosition(QTextCursor::StartOfLine);
					cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
					QString content = cursor.selectedText();
					cursor.insertText(QString(content.length(), ' '));
				}
				break;
			}
			case 1:
			{
				cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
				QString content = cursor.selectedText();
				cursor.insertText(QString(content.length(), ' '));
				qint32 firstLine = getFirstLineInViewport();
				qint32 currentLine = cursor.blockNumber();
				for (qint32 line = currentLine - 1; line >= firstLine; --line) {
					cursor.movePosition(QTextCursor::Up);
					cursor.movePosition(QTextCursor::StartOfLine);
					cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
					QString content = cursor.selectedText();
					cursor.insertText(QString(content.length(), ' '));
				}
				break;
			}
			case 2:
			{
				qint32 firstLine = getFirstLineInViewport();
				qint32 lastLine = getLastLineInViewport();
				for (qint32 line = firstLine; line <= lastLine; ++line) {
					cursor.movePosition(QTextCursor::Down);
					cursor.movePosition(QTextCursor::StartOfLine);
					cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
					QString content = cursor.selectedText();
					cursor.insertText(QString(content.length(), ' '));
				}
				break;
			}
			}
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'K': // 清除行, p1 参数为0清除光标位置到行末, 1清除行首到光标位置, 2清除整行
		{
			auto cursor = consoleView->textCursor();
			switch (content.toInt()) {
			case 0:
			{
				cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
				QString content = cursor.selectedText();
				cursor.insertText(QString(content.length(), ' '));
				break;
			}
			case 1:
			{
				cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
				QString content = cursor.selectedText();
				cursor.insertText(QString(content.length(), ' '));
				break;
			}
			case 2:
			{
				cursor.movePosition(QTextCursor::StartOfLine);
				cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
				QString content = cursor.selectedText();
				cursor.insertText(QString(content.length(), ' '));
				break;
			}
			}
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'L': // 在光标所在位置插入行, p1 行数
		{
			auto cursor = consoleView->textCursor();
			for (int i = 0; i < content.toInt(); ++i) {
				cursor.insertBlock();
			}
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'M': // 从光标所在位置开始删除行, p1 行数
		{
			auto cursor = consoleView->textCursor();
			for (int i = 0; i < content.toInt(); ++i) {
				cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
				cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
				cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
				cursor.removeSelectedText();
				cursor.deleteChar(); // 删除换行符
			}
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'P': // 删除光标所在位置及之后的字符, p1 个
		{
			auto cursor = consoleView->textCursor();
			for (int i = 0; i < content.toInt(); ++i) {
				cursor.deleteChar();
			}
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'S': // 向上滚动文字, p1 行数
		{
			for (int i = 0; i < content.toInt(); ++i) {
				consoleView->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
			}
			break;
		}
		case 'T': // 向下滚动文字, p1 行数
		{
			for (int i = 0; i < content.toInt(); ++i) {
				consoleView->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
			}
			break;
		}
		case 'X': // 从光标所在位置用空格替换字符, p1 个
		{
			auto cursor = consoleView->textCursor();
			for (int i = 0; i < content.toInt(); ++i) {
				cursor.deleteChar();
				cursor.insertText(" ");
			}
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'c': // 报告终端类型，回报格式为\033[?1;2c
		{
			emit q->inputPrepared("\033[?1;2c");
		}
		case 'd': // 移动光标到指定行, p1 行数
		{
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Start);
			qint32 lineNum = content.toInt();
			qint32 viewportFirstLine = getFirstLineInViewport();
			qint32 targetLine = viewportFirstLine + lineNum;
			qint32 currentLine = cursor.blockNumber();
			qint32 moveDelta = targetLine - currentLine;
			cursor.movePosition(moveDelta > 0 ? QTextCursor::Down : QTextCursor::Up, QTextCursor::MoveAnchor, std::abs(moveDelta));
			consoleView->setTextCursor(cursor);
			break;
		}
		case 'h': // 光标闪烁
		case 'l': // 光标显示
		case 'q': // 光标样式
		{
			// 这些命令不影响终端的显示内容，因此暂不支持
			break;
		}
		case 'm': // 颜色和格式，具体细节在Visindigo::Utility::Console中有所体现。
		{
			onColorAndFormatControlDetected(content);
			break;
		}
		case 'n': // 报告光标位置，回报格式为\033[行数;列数R
		{
			auto cursor = consoleView->textCursor();
			qint32 firstLine = getFirstLineInViewport();
			qint32 lineNum = cursor.blockNumber() + 1 - firstLine;
			qint32 colNum = cursor.positionInBlock() + 1;
			emit q->inputPrepared(QString("\033[%1;%2R").arg(lineNum).arg(colNum));
			break;
		}
		case 'r': // 设置滚动区域, p1 行数, p2 行数
		{
			// 为了实现便捷度考虑，目前滚动区域固定为视口，因此不支持修改滚动区域的命令
			break;
		}
		case 's': // 保存光标位置
		{
			savedCursorLine = consoleView->textCursor().blockNumber() - getFirstLineInViewport();
			savedCursorCol = consoleView->textCursor().positionInBlock();
			break;
		}
		case 'u': // 恢复光标位置
		{
			if (savedCursorLine != -1 && savedCursorCol != -1) {
				auto cursor = consoleView->textCursor();
				qint32 targetLine = getFirstLineInViewport() + savedCursorLine;
				qint32 delta = targetLine - cursor.blockNumber();
				cursor.movePosition(QTextCursor::StartOfLine);
				cursor.movePosition(delta > 0 ? QTextCursor::Down : QTextCursor::Up, QTextCursor::MoveAnchor, std::abs(delta));
				cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, savedCursorCol);
			}
			break;
		}
		case '@': // 在光标位置插入空格, p1 个
		{
			auto cursor = consoleView->textCursor();
			cursor.insertText(QString(content.toInt(), ' '));
			break;
		}
		case '=': // 小键盘应用模式
		case '>': // 小键盘数字模式
		{
			// 这两个命令不影响终端的显示内容，因此暂不支持
			break;
		}
		}
	}

	void TerminalPrivate::onColorAndFormatControlDetected(const QString& content) {
		QColor fgColor, bgColor;
		QTextCharFormat format;
		bool reset = false;
		bool bold = false;
		bool italic = false;
		bool underline = false;
		QStringList params = content.split(';');
		for (int i = 0; i < params.size(); i++) {
			int code = params[i].toInt();
			switch (code) {
			case 38:
				if (code == 38 && params.size() >= 5 && params[1] == "2" || i == 0) {
					int r = params[2].toInt();
					int g = params[3].toInt();
					int b = params[4].toInt();
					fgColor = QColor(r, g, b);
					i += 4;
				}
				break;
			case 48:
				if (code == 48 && params.size() >= 5 && params[1] == "2" || i == 0) {
					int r = params[2].toInt();
					int g = params[3].toInt();
					int b = params[4].toInt();
					bgColor = QColor(r, g, b);
					i += 4;
				}
				break;
			case 0:
				reset = true;
				break;
			case 1:
				bold = true;
				break;
			case 3:
				italic = true;
				break;
			case 4:
				underline = true;
				break;
			case 22:
				bold = false;
				break;
			case 24:
				underline = false;
				break;
			case 30:
				fgColor = QColor(Qt::black);
				break;
			case 31:
				fgColor = QColor(Qt::red);
				break;
			case 32:
				fgColor = QColor(Qt::green);
				break;
			case 33:
				fgColor = QColor(Qt::yellow);
				break;
			case 34:
				fgColor = QColor(Qt::blue);
				break;
			case 35:
				fgColor = QColor(Qt::magenta);
				break;
			case 36:
				fgColor = QColor(Qt::cyan);
				break;
			case 37:
				fgColor = QColor(Qt::white);
				break;
			case 39:
				fgColor = consoleView->palette().text().color(); // default.
				break;
			case 40:
				bgColor = QColor(Qt::black);
				break;
			case 41:
				bgColor = QColor(Qt::red);
				break;
			case 42:
				bgColor = QColor(Qt::green);
				break;
			case 43:
				bgColor = QColor(Qt::yellow);
				break;
			case 44:
				bgColor = QColor(Qt::blue);
				break;
			case 45:
				bgColor = QColor(Qt::magenta);
				break;
			case 46:
				bgColor = QColor(Qt::cyan);
				break;
			case 47:
				bgColor = QColor(Qt::white);
				break;
			case 49:
				bgColor = consoleView->palette().base().color(); // default.
				break;
			}
		}
		if (reset) {
			format = QTextCharFormat();
		}
		else {
			if (fgColor.isValid()) {
				format.setForeground(fgColor);
			}
			if (bgColor.isValid()) {
				format.setBackground(bgColor);
			}
			format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
			format.setFontItalic(italic);
			format.setUnderlineStyle(underline ? QTextCharFormat::SingleUnderline : QTextCharFormat::NoUnderline);
		}
		consoleView->setCurrentCharFormat(format);
	}
}
namespace Visindigo::Widgets {
	/*!
		\class Visindigo::Widgets::Terminal
		\brief 一个内置终端窗口，可以显示日志输出并接受用户输入的命令.
		\since Visindigo 0.13.0
		\inmodule Visindigo

		Terminal类提供了一个内置的终端窗口，可以显示日志输出并接受用户输入的命令。
		用户可以通过按下回车键或点击发送按钮来执行输入的命令。

		理论上，Terminal类还支持命令历史记录，用户可以通过上下箭头键来浏览之前输入的命令。

		\warning 这个类有已知缺陷：它并未完整实现。例如，命令历史记录功能可能存在问题，输入行的事件过滤器也可能不够完善。
		但它不会被移除，这个类的功能会逐步完善，直到它能够满足基本的终端需求为止。

		因此目前不为此类提供详细文档，也建议用户暂时不要使用。
	*/

	/*!
		\since Visindigo 0.13.0
		\a parent 父组件。

		构造函数
	*/
	Terminal::Terminal(QWidget* parent)
		: QFrame(parent), d(new Visindigo::__Private__::TerminalPrivate) {
		d->q = this;
		this->setWindowTitle("Visindigo Terminal");
		this->setMinimumSize(800, 600);
		QFont font("Cascadia Mono");
		d->consoleView = new QTextBrowser(this);
		d->consoleView->setLineWrapMode(QTextEdit::NoWrap);
		d->inputLine = new QLineEdit(this);
		d->inputLine->installEventFilter(d);
		d->consoleView->setFont(font);
		d->inputLine->setFont(font);
		d->layout = new QVBoxLayout(this);
		d->layout->addWidget(d->consoleView);
		d->layout->addWidget(d->inputLine);
		setLayout(d->layout);
		vgDebug << "Terminal initialized.";
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	Terminal::~Terminal() {
		delete d;
	}

	void Terminal::setInputEnable(bool enable) {
		d->enableInput = enable;
		d->inputLine->setEnabled(enable);
	}

	bool Terminal::isInputEnabled() const {
		return d->enableInput;
	}

	void Terminal::setMaxLines(qint32 lineCount) {
		d->maxLines = lineCount;
	}

	qint32 Terminal::getMaxLines() const {
		return d->maxLines;
	}

	void Terminal::clearConsole() {
		d->consoleView->clear();
	}

	void Terminal::addLine(const QString& line) {
		QString processedLine = line;
		if (d->workMode == WorkMode::PureText) {
			d->consoleView->append(processedLine);
		}
		else {
			d->onANSILineReceived(processedLine);
		}
	}
}