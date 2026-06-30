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
#include <QtWidgets/qapplication.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qregularexpression.h>
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
	void TerminalPrivate::appendANSICache(const QString& line, bool forceFlush) {
		cacheANSILine += line;
		if (forceFlush) {
			onANSILineReceived(cacheANSILine);
			cacheANSILine.clear();
		}
	}

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
				cursor.movePosition(QTextCursor::EndOfBlock);
				checkUrl(cursor);
				//如果下一行不存在，则创建一个新行
				if (not cursor.movePosition(QTextCursor::Down)) {
					cursor.insertBlock();
				}
				//光标移动到新行的行首
				cursor.movePosition(QTextCursor::StartOfLine);
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
		if (consoleView->document()->blockCount() > maxLines) {
			auto cursor = consoleView->textCursor();
			cursor.movePosition(QTextCursor::Start);
			for (int i = 0; i < consoleView->document()->blockCount() - maxLines; ++i) {
				cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
			}
			cursor.removeSelectedText();
			cursor.deleteChar(); // 删除多余的行后会剩下一个空行，这里删除掉
		}
	}

	void TerminalPrivate::insertPlainText(const QString& text) {
		if (text.isEmpty()) {
			return;
		}
		auto cursor = consoleView->textCursor();
		if (cursor.atBlockEnd()) {
			consoleView->insertPlainText(text);
		}
		else {
			cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, text.size());
			cursor.insertText(text);
			consoleView->setTextCursor(cursor);
		}
	}

	void TerminalPrivate::checkUrl(QTextCursor& cursor) {
		QTextBlock block = cursor.block();
		QString lineText = block.text();
		if (lineText.isEmpty()) return;

		QRegularExpression urlrx(R"(https?://\S+)");
		QRegularExpressionMatchIterator it = urlrx.globalMatch(lineText);
		if (!it.hasNext()) return;

		QTextDocument* doc = cursor.document();
		int blockPos = block.position();

		while (it.hasNext()) {
			QRegularExpressionMatch match = it.next();
			int start = match.capturedStart();
			int end = match.capturedEnd();
			QString url = match.captured(0);
			while (!url.isEmpty() && (url.back() == '.' || url.back() == ',' ||
				url.back() == ';' || url.back() == '?' ||
				url.back() == '!' || url.back() == ':')) {
				url.chop(1);
				end--;
			}
			if (url.isEmpty()) continue;
			QTextCursor urlCursor(doc);
			urlCursor.setPosition(blockPos + start);
			urlCursor.setPosition(blockPos + end, QTextCursor::KeepAnchor);
			QTextCharFormat currentFmt = urlCursor.charFormat();
			QTextCharFormat newFmt = currentFmt;
			newFmt.setAnchor(true);
			newFmt.setAnchorHref(url);

			// newFmt.setForeground(Qt::blue);
			// newFmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);

			urlCursor.setCharFormat(newFmt);
		}
	}


	void TerminalPrivate::onFixUpdate(double elapsedTime_ms) {
		if (not cacheANSILine.isEmpty()) {
			onANSILineReceived(cacheANSILine);
			cacheANSILine.clear();
		}
		if (ExternalProcess && ExternalProcess->state() == QProcess::Running) {
			QByteArray output = ExternalProcess->readAllStandardOutput();
			if (output.size() != 0) {
				QString text = QString::fromUtf8(output);
				q->addLine(text);
			}
			QByteArray error = ExternalProcess->readAllStandardError();
			if (error.size() != 0) {
				QString text = QString::fromUtf8(error);
				q->addLine(text);
			}
		}
		//qDebug() << "Terminal onFixUpdate, elapsedTime_ms: " << elapsedTime_ms; 
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
			break;
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
				if (code == 38 && params.size() >= 5 && params[1] == "2" && i == 0) {
					int r = params[2].toInt();
					int g = params[3].toInt();
					int b = params[4].toInt();
					fgColor = QColor(r, g, b);
					i += 4;
				}
				break;
			case 48:
				if (code == 48 && params.size() >= 5 && params[1] == "2" && i == 0) {
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

	bool TerminalPrivate::eventFilter(QObject* obj, QEvent* event) {
		if (obj == inputLine) {
			if (event->type() == QEvent::KeyPress) {
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
				if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
					QString command = inputLine->text();
					commandHistory.append(command);
					if (commandHistory.size() > maxCommandHistory) {
						commandHistory.removeFirst();
					}
					inputLine->clear();
					emit q->inputPrepared(command);
					return true;
				}
				else if (keyEvent->key() == Qt::Key_Up) {
					if (historyQueryDelta == 0) {
						currentHistory = inputLine->text();
					}
					historyQueryDelta++;
					inputLine->setText(commandHistory.value(commandHistory.size() - historyQueryDelta, currentHistory));
				}
				else if (keyEvent->key() == Qt::Key_Down) {
					if (historyQueryDelta > 0) {
						historyQueryDelta--;
						inputLine->setText(commandHistory.value(commandHistory.size() - historyQueryDelta, currentHistory));
					}
					else {
						inputLine->setText(currentHistory);
					}
				}
				else {
					currentHistory = inputLine->text();
					historyQueryDelta = 0;
				}
			}
		}
		return QObject::eventFilter(obj, event);
	}
}
namespace Visindigo::Widgets {
	/*!
		\class Visindigo::Widgets::Terminal
		\brief 一个内置终端窗口，可以显示日志输出并接受用户输入的命令.
		\since Visindigo 0.13.0
		\inmodule Visindigo

		Terminal类提供了一个内置的终端窗口，可以显示日志输出并接受用户输入的命令。
		用户可以通过按下回车键或点击发送按钮来执行输入的命令。稍后，inputPrepared信号将被触发，携带用户输入的命令文本，供外部处理。

		自0.16.0开始，这类自带一个外部进程管理。通过launchExternalProcess方法，用户可以在终端中启动一个外部进程，
		并将该进程的标准输出和标准错误输出重定向到终端窗口中显示。同时，用户在终端中输入的命令也会被发送到该外部进程的标准输入。
		这使得Terminal类不仅可以作为一个简单的日志窗口，还可以作为一个功能完整的终端模拟器来使用。

		理论上，Terminal类还支持命令历史记录，用户可以通过上下箭头键来浏览之前输入的命令。

		\section1 性能与实时性
		Terminal类的设计目标是提供一个功能完善的终端窗口，能够正确处理ANSI控制序列并显示丰富的文本格式。
		然而，由于Qt的文本渲染机制和事件处理机制，Terminal类的解析性能相对较差，且只能在事件循环正常
		运行时刷新显示内容。这就意味着在主线程的某个循环体内大量输出日志时，实际上并不会有东西显示出来。

		Terminal类在早期曾经尝试过允许用户设置缓冲区与内部调用qApp->processEvent以缓解性能问题并
		解决实时性，但内部调用processEvent可能会破坏原有的事件循环封装，带来难以预料的问题。因此基于
		这种难以达到实时性的考虑，我们退而求其次也取消了缓冲区设计，转为每秒固定尝试刷新显示内容
		20次。

		这种摆烂设计的一个重要好处是不会破坏事件循环封装，也不会影响循环体的执行性能。如果用户真的
		需要在中途刷新显示内容，应当自行尝试使用qApp->processEvent或类似的方式，
		但这需要用户自己权衡性能与实时性，并且需要注意可能带来的副作用。
	*/

	/*!
		\since Visindigo 0.16.0
		\enum Visindigo::Widgets::Terminal::WorkMode
		\value PureText 纯文本模式，终端将不解析任何ANSI控制序列，所有输入都将被视为普通文本直接显示。这种模式适用于只需要简单日志输出而不需要格式控制的场景。
		\value ANSIControl ANSI控制序列模式，终端将解析输入中的ANSI控制序列以实现丰富的文本格式和控制效果。这是默认模式，适用于需要完整终端功能的场景。

		如果你确定输出的内容不会有任何ANSI序列控制，则PureText模式会有更好的性能表现。
		与此同时，PureText下，每次addLine时都直接将文本添加到终端中，而不需要等待定时刷新。
	*/

	/*!
		\fn void Visindigo::Widgets::Terminal::inputPrepared(const QString& command)
		\since Visindigo 0.16.0

		当用户在输入框中输入命令并按下回车键时，inputPrepared信号将被触发，携带用户输入的命令文本，供外部处理。
		
		input函数也会触发此信号。
	*/

	/*!
		\fn void Visindigo::Widgets::Terminal::externalProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
		\since Visindigo 0.16.0
		
		当通过launchExternalProcess启动的外部进程结束时，externalProcessFinished信号将被触发，携带外部进程的退出代码和退出状态。
	*/

	/*!
		\fn void Visindigo::Widgets::Terminal::stdoutReceived(const QString& text)
		\since Visindigo 0.16.0

		当通过launchExternalProcess启动的外部进程有新的标准输出内容时，stdoutReceived信号将被触发，携带新输出的文本内容。
	*/

	/*!
		\fn void Visindigo::Widgets::Terminal::stderrReceived(const QString& text)
		\since Visindigo 0.16.0

		当通过launchExternalProcess启动的外部进程有新的标准错误输出内容时，stderrReceived信号将被触发，携带新输出的文本内容。
	*/

	/*!
		\since Visindigo 0.16.0

		在Windows平台上，创建一个预配置为使用PowerShell的Terminal实例。

		它最大的便捷之处在于提前将代码页切换到65001（UTF-8），这样就不需要担心中文乱码问题了。

		在其他平台，这返回空指针。
	*/
	Terminal* Terminal::createPowerShell() {
#ifdef Q_OS_WIN
		Terminal* terminal = new Terminal;
		terminal->launchExternalProcess("powershell.exe", QStringList() << "-NoExit" << "-Command" << "chcp 65001 > $null");
		return terminal;
#else
		return nullptr;
#endif
	}
	/*!
		\since Visindigo 0.13.0
		\a parent 父组件。

		构造函数
	*/
	Terminal::Terminal(QWidget* parent)
		: QFrame(parent), d(new Visindigo::__Private__::TerminalPrivate) {
		d->q = this;
		this->setWindowTitle("Visindigo Terminal");
		this->resize(800, 600);
		QFont font("Cascadia Mono");
		font.setPointSizeF(qApp->font().pointSizeF() * 0.8);
		this->setContentsMargins(10, 10, 10, 10);
		d->consoleView = new QTextBrowser(this);
		d->consoleView->setLineWrapMode(QTextEdit::NoWrap);
		d->consoleView->setOpenExternalLinks(true);
		d->inputLine = new QLineEdit(this);
		d->inputLine->installEventFilter(d);
		d->consoleView->setFont(font);
		d->inputLine->setFont(font);
		d->layout = new QVBoxLayout(this);
		d->layout->setContentsMargins(0, 0, 0, 0);
		d->layout->addWidget(d->consoleView);
		d->layout->addWidget(d->inputLine);
		setLayout(d->layout);
		d->inputLine->installEventFilter(d);
		d->setUpdateType(Visindigo::General::TickObject::FixUpdate);
		d->setFixUpdateInterval(Visindigo::General::TickObject::FPS_20);
		d->enableUpdate();
		vgDebug << "Terminal initialized.";
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	Terminal::~Terminal() {
		if (d->ExternalProcess && d->ExternalProcess->state() == QProcess::Running) {
			if (not d->detachWhenTerminalClosed){
				d->ExternalProcess->kill();
				d->ExternalProcess->waitForFinished();
			}
			else {
				d->ExternalProcess->setParent(nullptr);
			}
		}
		d->disableAndDelete();
	}

	/*!
		\since Visindigo 0.13.0
		\a enable 是否启用输入框。

		设置输入框是否启用。
	*/
	void Terminal::setInputEnable(bool enable) {
		d->enableInput = enable;
		d->inputLine->setEnabled(enable);
	}

	/*!
		\since Visindigo 0.13.0

		return 输入框是否启用。
	*/
	bool Terminal::isInputEnabled() const {
		return d->enableInput;
	}

	/*!
		\since Visindigo 0.13.0

		return 命令历史记录。
	*/
	QStringList Terminal::getCommandHistory() const {
		return d->commandHistory;
	}

	/*!
		\since Visindigo 0.16.0
		\a historyList 命令历史记录列表。

		设置命令历史记录列表。这个函数会替换掉原有的命令历史记录，并且会将历史记录数量限制在当前设置的最大数量之内。

		结合get函数，你可以有办法持久化存储命令历史记录，例如在程序关闭时保存到文件中，在程序启动时加载回来。
	*/
	void Terminal::setCommandHistory(const QStringList& historyList) {
		d->commandHistory = historyList;
		while (d->commandHistory.size() > d->maxCommandHistory) {
			d->commandHistory.removeFirst();
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a historyCount 历史记录的最大数量。

		设置命令历史记录的最大数量。当历史记录超过这个数量时，最早的记录将被删除。
	*/
	void Terminal::setMaxCommandHistory(qint32 historyCount) {
		d->maxCommandHistory = historyCount;
		while (d->commandHistory.size() > d->maxCommandHistory) {
			d->commandHistory.removeFirst();
		}
	}

	/*!
		\since Visindigo 0.13.0

		return 命令历史记录的最大数量。
	*/
	qint32 Terminal::getMaxCommandHistory() const {
		return d->maxCommandHistory;
	}

	/*!
		\since Visindigo 0.13.0
		\a autoScroll 是否启用自动滚动。

		设置是否启用自动滚动。当启用时，终端会在输出新内容时自动滚动到最新行。

		目前，只要有新行被添加，就强制滚动到最新。
	*/
	void Terminal::setAutoScroll(bool autoScroll) {
		d->autoScroll = autoScroll;
	}
	
	/*!
		\since Visindigo 0.13.0

		return 是否启用自动滚动。
	*/
	bool Terminal::isAutoScroll() const {
		return d->autoScroll;
	}

	/*!
		\since Visindigo 0.13.0
		\a lineCount 最大行数。

		设置终端缓存的最大行数。当终端中的行数超过这个数量时，最早的行将被删除以保持行数不超过这个限制。
	*/
	void Terminal::setMaxLines(qint32 lineCount) {
		d->maxLines = lineCount;
	}

	/*!
		\since Visindigo 0.13.0

		return 终端缓存的最大行数。
	*/
	qint32 Terminal::getMaxLines() const {
		return d->maxLines;
	}

	/*!
		\since Visindigo 0.13.0

		 清除终端中的所有内容。
	*/
	void Terminal::clearConsole() {
		d->consoleView->clear();
	}

	/*!
		\since Visindigo 0.16.0
		\a line 要输入的命令文本。

		模拟从输入框直接输入命令的效果，基本上等于直接触发inputPrepared信号，但也会将输入的命令添加到命令历史中。
		如果输入框中已有内容，它不会干扰已有内容和历史记录的浏览。
	*/
	void Terminal::input(const QString& line) {
		d->commandHistory.append(line);
		if (d->commandHistory.size() > d->maxCommandHistory) {
			d->commandHistory.removeFirst();
		}
		emit inputPrepared(line);
	}

	/*!
		\since Visindigo 0.16.0
		\a line 要添加的行文本。
		\a forceFlush 是否强制刷新显示内容。
		向终端添加一行文本。该函数会正确处理ANSI控制序列以实现丰富的文本格式和控制效果。

		如果forceFlush参数为true，函数会立即刷新显示内容，否则会等待下一次定时刷新。

		请注意，这个forceFlush参数的立即刷新仅仅是将缓冲区中的内容进行解析，但不保证
		显示内容也立即更新，因为Qt的显示更新仍然需要等到事件循环处理时才会进行。
	*/
	void Terminal::addLine(const QString& line, bool forceFlush ) {
		QString processedLine = line;
		if (d->workMode == WorkMode::PureText) {
			d->consoleView->append(processedLine);
		}
		else {
			d->appendANSICache(line, forceFlush);
		}
	}

	/*!
		\since Visindigo 0.16.0
		\a mode 工作模式。

		 设置终端的工作模式。不同的工作模式会影响终端如何处理输入的文本以及如何显示内容。
	*/
	void Terminal::setWorkMode(WorkMode mode) {
		d->workMode = mode;
	}

	/*!
		\since Visindigo 0.16.0

		 return 终端当前的工作模式。
	*/
	Terminal::WorkMode Terminal::getWorkMode() const {
		return d->workMode;
	}

	/*!
		\since Visindigo 0.16.0
		\a utf8 设置是否以UTF-8编码对待外部进程，默认为true。

		设置是否以UTF-8编码对待通过launchExternalProcess启动的外部进程。当设置为true时，
		终端会将外部进程的标准输出和标准错误输出视为UTF-8编码的文本进行解析和显示；
		并将输入的命令文本以UTF-8编码发送到外部进程的标准输入。

		当设置为false时，终端会使用系统默认的本地代码页（所谓Local8Bit）来解析输出并传递输入。

		你可以随时更改它，更改立即生效，并且会影响后续的输入输出，但不会影响已经显示的内容。
	*/
	void Terminal::setExternalProcessUTF8(bool utf8) {
		d->externalProcessUTF8 = utf8;
	}

	/*!
		\since Visindigo 0.16.0

		return 是否以UTF-8编码对待外部进程。
	*/
	bool Terminal::isExternalProcessUTF8() const {
		return d->externalProcessUTF8;
	}

	/*!
		\since Visindigo 0.16.0
		\a systemCommand 要执行的系统命令，用于启动外部进程
		\a arguments 外部进程的命令行参数
		\a workingDirectory 外部进程的工作目录，默认为空表示使用当前目录

		Terminal使用 \a systemCommand 启动一个外部进程，并将该进程的标准输出和标准错误输出重定向到终端窗口中显示。
		同时，用户在终端中输入的命令也会被发送到该外部进程的标准输入。

		请注意，如果已有外部进程在运行，这个函数不做任何事情
	*/
	void Terminal::launchExternalProcess(const QString& systemCommand, const QStringList& arguments, const QString& workingDirectory) {
		if (not d->ExternalProcess) {
			d->ExternalProcess = new QProcess(this);
			connect(d->ExternalProcess, &QProcess::readyReadStandardOutput, this, [this]() {
				QByteArray output = d->ExternalProcess->readAllStandardOutput();
				QString text;
				if (not d->externalProcessUTF8) {
					text = QString::fromLocal8Bit(output);
				}
				else {
					text = QString::fromUtf8(output);
				}
				emit stdoutReceived(text);
				//vgDebug << "External process stdout: " << text.replace("\033", "\\033");
				addLine(text);
				});
			connect(d->ExternalProcess, &QProcess::readyReadStandardError, this, [this]() {
				QByteArray output = d->ExternalProcess->readAllStandardError();
				QString text;
				if (not d->externalProcessUTF8) {
					text = QString::fromLocal8Bit(output);
				}
				else {
					text = QString::fromUtf8(output);
				}
				emit stderrReceived(text);
				addLine(text);
				});
			connect(this, &Terminal::inputPrepared, this, [this](const QString& command) {
				if (d->ExternalProcess && d->ExternalProcess->state() != QProcess::NotRunning) {
					if (not d->externalProcessUTF8) {
						d->ExternalProcess->write((command + "\n").toLocal8Bit());
					}
					else {
						d->ExternalProcess->write((command + "\n").toUtf8());
					}
				}
				});
			connect(d->ExternalProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
				emit externalProcessFinished(exitCode, exitStatus);
				});
		}
		if (d->ExternalProcess->state() == QProcess::NotRunning) {
			QString workingDir = workingDirectory.isEmpty() ? QDir::currentPath() : workingDirectory;
			d->ExternalProcess->setWorkingDirectory(workingDir);
			d->ExternalProcess->start(systemCommand, arguments);
		}
	}

	/*!
		\since Visindigo 0.16.0
		 
		 return 外部进程是否正在运行
	*/
	bool Terminal::isExternalProcessRunning() const {
		return d->ExternalProcess && d->ExternalProcess->state() != QProcess::NotRunning;
	}

	/*!
		\since Visindigo 0.16.0
		\a waiting 是否等待程序终止

		终止正在运行的外部进程
	*/
	void Terminal::terminateExternalProcess(bool waiting) {
		if (d->ExternalProcess && d->ExternalProcess->state() != QProcess::NotRunning) {
			d->ExternalProcess->terminate();
			if (waiting) {
				d->ExternalProcess->waitForFinished();
			}
		}
	}

	/*!
		\since Visindigo 0.16.0

		return 外部进程的QProcess对象，在没有调用过launchExternalProcess的情况下，可能返回nullptr
	*/
	QProcess* Terminal::getExternalProcess() const {
		return d->ExternalProcess;
	}
}