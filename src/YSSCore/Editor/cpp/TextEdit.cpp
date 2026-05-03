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
#include <QtCore/qregularexpression.h>
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
					if (keyEvent->modifiers() & Qt::AltModifier && keyEvent->modifiers() & Qt::ControlModifier) {
						vgDebug << "Alt + Ctrl + Direction Key Pressed";
						onAltMultiSelection(keyEvent);
						return true;
					}
					else if (keyEvent->modifiers() & Qt::AltModifier) {
						vgDebug << "Alt + Direction Key Pressed";
						onAltSwapLine(keyEvent);
						return true;
					}
					else {
						clearAltMultiSelection();
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
				}
				else if (keyEvent->key() == Qt::Key_D && keyEvent->modifiers() & Qt::ControlModifier) {
					onDoubleLine(keyEvent);
					return true;
				}
				else if (keyEvent->key() == Qt::Key_F && keyEvent->modifiers() & Qt::ControlModifier) {
					showFindAndReplace();
					return true;
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
			if (event->type() == QEvent::Hide || event->type() == QEvent::HideToParent) {
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
			else if (event->type() == QEvent::Resize) {
				FindAndReplaceWidget->move(q->width() - 412, 0); // 400 for width, 12 for vertical scrollbar width
			}
		}
		return false;
	}

	void TextEditPrivate::onBlockCountChanged(qint32 count) {
		CursorInfo->setTotalLines(count);
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
			if (TabCompleterWidget != nullptr && TabCompleterWidget->isVisible()) {
				TabCompleterWidget->hide();
			}
			if (HoverInfoWidget != nullptr && HoverInfoWidget->isVisible()) {
				HoverInfoWidget->hide();
			}
			useKeyboardToMoveCursor = false;
			return;
		}
		onCompleter();
		onHoverInfo(false);
		QTextCursor cursor = Text->textCursor();
		int index = cursor.block().blockNumber();
		int column = cursor.positionInBlock();
		int selection = cursor.selectedText().length();
		CursorInfo->setCursorInfo(index + 1, column + 1, selection);
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
		if (cursor.hasSelection()) {
			return;
		}
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
				bool rightOut = pos.x() + TabCompleterWidget->width() > Text->viewport()->width();
				if (rightOut) {
					//right align
					TabCompleterWidget->move(QPoint(pos.x() + 10 - TabCompleterWidget->width(), pos.y() + 20));
				}
				else {
					// left align
					TabCompleterWidget->move(QPoint(pos.x() + 10, pos.y() + 20));
				}
			}
			else {
				bool rightOut = pos.x() + TabCompleterWidget->width() > HoverArea->width();
				if (rightOut) {
					//right align
					TabCompleterWidget->move(Text->mapTo(HoverArea, QPoint(pos.x() + 10 - TabCompleterWidget->width(), pos.y() + 20)));
				}
				else {
					// left align
					TabCompleterWidget->move(Text->mapTo(HoverArea, QPoint(pos.x() + 10, pos.y() + 20)));
				}
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
				cursor.beginEditBlock();
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
				cursor.endEditBlock();
			}
			else {
				//从行首拉到光标位置
				QTextCursor cursor = Text->textCursor();
				cursor.beginEditBlock();
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
				cursor.endEditBlock();
			}
		}
		else {
			if (Text->textCursor().hasSelection()) {
				QTextCursor cursor = Text->textCursor();
				//判断cursor的行数量
				int lineCount = cursor.selectedText().count(QChar(0x2029)) + 1;
				bool reverse = cursor.position() == cursor.selectionEnd();
				cursor.beginEditBlock();
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
				cursor.endEditBlock();
			}
			else {
				QTextCursor cursor = Text->textCursor();
				cursor.beginEditBlock();
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
				cursor.endEditBlock();
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
		if (FindAndReplaceWidget != nullptr && FindAndReplaceWidget->isVisible()) {
			// I think this function should be moved to farWidget.hideEvent().
			// Currently, different hide operations do not share same code, which is not good.
			clearFindAllMultiSelection(); 
			FindAndReplaceWidget->hide();
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
		if (dist > 10.0f) { // only reset timer when mouse moved enough distance
			HoverTimer->start(HoverTimeout);
			LastMousePos = event->pos();
		}
		else {
			return;
		}
		HoverTimer->start(HoverTimeout);
		if (HoverInfoWidget != nullptr && HoverInfoWidget->isVisible()) {
			HoverInfoWidget->hide();
		}
	}

	bool TextEditPrivate::onMouseScroll(QWheelEvent* event) {
		if (HoverInfoWidget && HoverInfoWidget->isVisible()) {
			// scroll the hover info widget
			HoverInfoWidget->scrollBy(-event->angleDelta().y());
			return true;
		}
		else if (TabCompleterWidget && TabCompleterWidget->isVisible()) {
			TabCompleterWidget->scrollBy(-event->angleDelta().y());
			return true;
		}else{
			return false;
		}
	}

	void TextEditPrivate::onHoverTimeout() {
		onHoverInfo(true);
	}

	void TextEditPrivate::onHoverInfo(bool triggerFromHover) {
		if (not triggerFromHover && Text->textCursor().hasSelection()) {
			return;
		}
		if (HoverInfoProvider != nullptr) {
			QPoint pos = QCursor::pos();
			HoverInfoProvider->d->HoverSetSth = false;
			if (triggerFromHover) {
				if (!Text->viewport()->rect().contains(Text->mapFromGlobal(pos))) {
					HoverTimer->stop();
					HoverInfoWidget->hide();
					return;
				}
			}
			HoverInfoProvider->d->TriggerFromHover = triggerFromHover;
			QTextCursor cursor;
			if (triggerFromHover) {
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
					bool rightOut = tpos.x() + tpos.width() + HoverInfoWidget->width() > Text->viewport()->width();
					if (rightOut) {
						// right align
						HoverInfoWidget->move(tpos.x() - HoverInfoWidget->width(), tpos.y());
					}
					else {
						// left align
						HoverInfoWidget->move(tpos.x() + tpos.width(), tpos.y());
					}
				}
				else {
					QRect lpos = Text->cursorRect(cursor);
					if (!HoverArea) {
						bool rightOut = lpos.x() + HoverInfoWidget->width() > Text->viewport()->width();
						if (rightOut) {
							// right align
							HoverInfoWidget->move(QPoint(lpos.x() + 10 - HoverInfoWidget->width(), lpos.y() + 20));
						}
						else {
							// left align
							HoverInfoWidget->move(QPoint(lpos.x() + 10, lpos.y() + 20));
						}
					}
					else {
						bool rightOut = lpos.x() + HoverInfoWidget->width() > HoverArea->width();
						if (rightOut) {
							// right align
							HoverInfoWidget->move(Text->mapTo(HoverArea, QPoint(lpos.x() + 10 - HoverInfoWidget->width(), lpos.y() + 20)));
						}
						else {
							// left align
							HoverInfoWidget->move(Text->mapTo(HoverArea, QPoint(lpos.x() + 10, lpos.y() + 20)));
						}
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
			if (not HoverArea) {
				bool rightOut = pos.x() + TabCompleterWidget->width() > Text->viewport()->width();
				if (rightOut) {
					//right align
					TabCompleterWidget->move(QPoint(pos.x() + 10 - TabCompleterWidget->width(), pos.y() + 20));
				}
				else {
					// left align
					TabCompleterWidget->move(QPoint(pos.x() + 10, pos.y() + 20));
				}
			}
			else {
				bool rightOut = pos.x() + TabCompleterWidget->width() > HoverArea->width();
				if (rightOut) {
					//right align
					TabCompleterWidget->move(Text->mapTo(HoverArea, QPoint(pos.x() + 10 - TabCompleterWidget->width(), pos.y() + 20)));
				}
				else {
					// left align
					TabCompleterWidget->move(Text->mapTo(HoverArea, QPoint(pos.x() + 10, pos.y() + 20)));
				}
			}
		}
	}

	void TextEditPrivate::onAltSwapLine(QKeyEvent* event) {
		if (event->key() == Qt::Key_Up && Text->textCursor().block().blockNumber() == 0) {
			return; // already the first line, can't move up
		}
		else if (event->key() == Qt::Key_Down && Text->textCursor().block().blockNumber() == Text->document()->blockCount() - 1) {
			return; // already the last line, can't move down
		}
		if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
			QTextCursor cursor = Text->textCursor();
			qint32 currentColumn = cursor.positionInBlock();
			QString currentLine = cursor.block().text();
			QTextBlock swapBlock = event->key() == Qt::Key_Up ? cursor.block().previous() : cursor.block().next();
			QString swapLine = swapBlock.text();

			cursor.setPosition(cursor.block().position());
			cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
			cursor.insertText(swapLine);

			cursor.setPosition(swapBlock.position());
			cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
			cursor.insertText(currentLine);

			cursor.setPosition(swapBlock.position() + currentColumn);
			Text->setTextCursor(cursor);
			useKeyboardToMoveCursor = true;
		}
	}

	void TextEditPrivate::onDoubleLine(QKeyEvent* event) {
		QTextCursor cursor = Text->textCursor();
		qint32 currentColumn = cursor.positionInBlock();
		QString currentLine = cursor.block().text();
		cursor.movePosition(QTextCursor::EndOfBlock);
		cursor.insertText("\n" + currentLine);
		cursor.setPosition(cursor.block().position() + currentColumn);
		Text->setTextCursor(cursor);
		useKeyboardToMoveCursor = true;
	}

	void TextEditPrivate::onAltMultiSelection(QKeyEvent* event) {
		/*
			This function is disabled for now, as it has multi issues and not completely implemented.
			currenty, the ExtraSelection function is  used by FindAll, and it will cause conflict if we
			enable AltMultiSelection. A middle layer should be designed to manage multiple ExtraSelection sources,
			and merge them together before applying to the TextEdit.
		*/
		return;

		if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
			if (AltMultiSelections.isEmpty()) {
				QTextCursor cursor = Text->textCursor();
				QTextEdit::ExtraSelection selection;
				selection.cursor = cursor;
				selection.format.setBackground(VISTM->getColor("SelectedTextBackground").lighter(150));
				AltMultiSelections.append(selection);
			}
			if (event->key() == Qt::Key_Up) {
				QTextCursor cursor = AltMultiSelections.first().cursor;
				cursor.movePosition(QTextCursor::Up);
				QTextEdit::ExtraSelection selection;
				selection.cursor = cursor;
				selection.format.setBackground(VISTM->getColor("SelectedTextBackground").lighter(150));
				AltMultiSelections.prepend(selection);
			}
			else {
				QTextCursor cursor = AltMultiSelections.last().cursor;
				cursor.movePosition(QTextCursor::Down);
				QTextEdit::ExtraSelection selection;
				selection.cursor = cursor;
				selection.format.setBackground(VISTM->getColor("SelectedTextBackground").lighter(150));
				AltMultiSelections.append(selection);
			}
			Text->setExtraSelections(AltMultiSelections);
		}
		else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
			if (AltMultiSelections.isEmpty()) {
				return;
			}
			for (int i = 0; i < AltMultiSelections.size(); i++) {
				QTextCursor cursor = AltMultiSelections[i].cursor;
				cursor.movePosition(event->key() == Qt::Key_Left ? QTextCursor::Left : QTextCursor::Right, QTextCursor::KeepAnchor);
				AltMultiSelections[i].cursor = cursor;
			}
			Text->setExtraSelections(AltMultiSelections);
		}
	}

	void TextEditPrivate::clearAltMultiSelection() {
		if (!AltMultiSelections.isEmpty()) {
			AltMultiSelections.clear();
			Text->setExtraSelections(AltMultiSelections);
		}
	}

	void TextEditPrivate::createFindAllMultiSelection(QList<QTextCursor> findResults) {
		FindAllMultiSelections.clear();
		for (int i = 0; i < findResults.size(); i++) {
			QTextEdit::ExtraSelection selection;
			selection.cursor = findResults[i];
			selection.format.setBackground(VISTM->getColor("SelectedTextBackground").lighter(150));
			FindAllMultiSelections.append(selection);
		}
		Text->setExtraSelections(FindAllMultiSelections);
	}

	void TextEditPrivate::clearFindAllMultiSelection() {
		if (!FindAllMultiSelections.isEmpty()) {
			FindAllMultiSelections.clear();
			Text->setExtraSelections(FindAllMultiSelections);
		}
	}

	void TextEditPrivate::showFindAndReplace() {
		if (Text->textCursor().hasSelection()) {
			QString selectedText = Text->textCursor().selectedText();
			selectedText.replace(QChar::ParagraphSeparator, "\n");
			FindAndReplaceWidget->setFindText(selectedText);
		}
		FindAndReplaceWidget->show();
	}
}

namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::TextEdit
		\brief 这是YSS最关键的功能：代码编辑器.
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup LangService FileService
		TextEdit是Yayin Story Studio中最关键、最核心的功能，即代码编辑器。

		此类相比于Qt提供的QTextEdit，额外提供了其所缺失的现代代码编辑必备的几项基本功能：
		\list
			\li 自动缩进
			\li 多行Tab的缩进和反缩进
			\li 行号
			\li 鼠标悬停提示、Tab补全
		\endlist

		基本上讲，以上这些额外补充的功能就已经满足了一个现代代码编辑器的基本需求。

		\section1 语言服务
		由于TextEdit是设计为Yayin Story Studio的内置代码编辑器，因此这个类的耦合性要稍微高一些。如果要
		提供语言语法支持，就不能单独使用这个类，它会通过访问YSSCore::Editor::LangServerManager
		来获取语言服务器的支持，并且在编辑器中提供语法高亮、鼠标悬停提示、Tab补全等功能。

		因此，如果要为TextEdit提供语言支持，您需要首先实现一个继承自YSSCore::Editor::LangServer的类，
		并且在其中实现语法高亮、鼠标悬停提示、Tab补全等功能。然后，您需要将这个语言服务器注册到YSSCore::Editor::LangServerManager中，
		TextEdit会自动按扩展名选择合适的语言服务器来提供支持。

		\section1 标准文件编辑框架
		为了统一Yayin Story Studio中全部的打开文件->路由合适的打开方式->编辑->保存的留存，该类
		继承自YSSCore::Editor::FileEditWidget。并在YSSCore::Editor::FileServerManager中作为默认的通用文件打开方式使用。

		\note 一般来说，TextEdit单独使用或组合使用，不应该继承此类，但考虑到潜在需求，因此也未设置final。

		\warning 这类在0.14.0版本调整过虚函数，ABI完全不兼容。
	*/

	/*!
		\since YSS 0.13.0
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
		d->Text->setAcceptRichText(false);
		this->installEventFilter(d);
		d->Layout = new QGridLayout(this);
		d->Layout->addWidget(d->Line, 0, 0);
		d->Layout->addWidget(d->Text, 0, 1);
		d->Layout->setSpacing(0);
		d->Layout->setContentsMargins(0, 0, 0, 0);
		d->CursorInfo = new YSSCore::__Private__::TextEditCursorInfo(this);
		d->CursorInfo->setFixedHeight(30);
		d->CursorInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		//testLabel->setFrameStyle(QFrame::Box | QFrame::Plain);
		d->Layout->addWidget(d->CursorInfo, 1, 0, 1, 2);

		d->LastCursor = d->Line->textCursor();
		d->LastCursor.movePosition(QTextCursor::Start);
		d->LastCursorLine = 0;

		d->HoverTimer = new QTimer(this);
		d->HoverTimer->setInterval(d->HoverTimeout);

		d->FindAndReplaceWidget = new YSSCore::__Private__::TextEditFindAndReplace(this);
		d->FindAndReplaceWidget->resize(400, 200);
		d->FindAndReplaceWidget->move(this->width() - 412, 0);
		d->FindAndReplaceWidget->hide();
		connect(d->Text->document(), &QTextDocument::blockCountChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onBlockCountChanged);
		connect(d->Text->verticalScrollBar(), &QScrollBar::valueChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onScrollBarChanged);
		connect(d->Line->verticalScrollBar(), &QScrollBar::valueChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onScrollBarChanged);
		connect(d->Text, &QTextEdit::cursorPositionChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onCursorPositionChanged);
		connect(d->HoverTimer, &QTimer::timeout, this->d, &YSSCore::__Private__::TextEditPrivate::onHoverTimeout);
	}
	/*!
		\since YSS 0.13.0
		TextEdit的析构函数。
	*/
	TextEdit::~TextEdit() {
		// d->Text must be delete before d :
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
		\since YSS 0.13.0
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
		\since YSS 0.13.0
		设置TextEdit中的文本内容。此函数会替换掉TextEdit中原有的全部文本内容。
	*/
	void TextEdit::setPlainText(const QString& text) {
		d->Text->setPlainText(text);
		this->setFileChanged();
	}

	/*!
		\since YSS 0.13.0
		获取TextEdit中的文本内容。此函数会返回TextEdit中全部的文本内容。
	*/
	QString TextEdit::getPlainText() const {
		return d->Text->toPlainText();
	}

	/*!
		\since YSS 0.13.0
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
		\since YSS 0.13.0
		获取当前光标所在的行号。行号从1开始。
	*/
	int TextEdit::getCurrentLineNumber() const {
		QTextCursor cursor = d->Text->textCursor();
		return cursor.block().blockNumber() + 1;
	}

	/*!
		\since YSS 0.13.0
		设置鼠标悬停提示的超时时间。单位为毫秒。
	*/
	void TextEdit::setHoverTimeout(qint32 ms) {
		d->HoverTimeout = ms;
		d->HoverTimer->setInterval(ms);
	}

	/*!
		\since YSS 0.13.0
		获取鼠标悬停提示的超时时间。单位为毫秒。
	*/
	qint32 TextEdit::getHoverTimeout() const {
		return d->HoverTimeout;
	}

	/*!
		\since YSS 0.13.0
		设置Tab键是否使用空格进行缩进。默认为false，即使用制表符进行缩进。
	*/
	void TextEdit::setTabReload(bool reload) {
		d->ReloadTab = reload;
	}

	/*!
		\since YSS 0.13.0
		获取Tab键是否使用空格进行缩进。
	*/
	bool TextEdit::isTabReload() const {
		return d->ReloadTab;
	}

	/*!
		\since YSS 0.13.0
		设置Tab键的缩进宽度。单位为字符数。默认为4。
	*/
	QTextDocument* TextEdit::getDocument() const {
		return d->Text->document();
	}

	/*!
		\since Visindigo 0.14.0
		获取字体的字号。
	*/
	qint32 TextEdit::getFontSize() const {
		return d->Text->font().pointSize();
	}

	/*!
		\since Visindigo 0.14.0
		设置字体的字号。
	*/
	void TextEdit::setFontSize(qint32 size) {
		QFont font = d->Text->font();
		font.setPointSize(size);
		d->Text->setFont(font);
		d->Line->setFont(font);
		double rawSpaceWidth = d->FontMetrics->size(Qt::TextSingleLine, " ").width();
		double tabStopDistance = d->TabWidth * rawSpaceWidth * this->devicePixelRatioF();
		d->Text->setTabStopDistance(qMax(20.0, tabStopDistance));
	}

	/*!
		\since YSS 0.14.0
		在整个文档中查找\a source。当\a sourceAsRe为真时，将\a source视作正则表达式进行查找。
		返回一个包含所有匹配结果的QTextCursor构成的列表。这些QTextCursor的特性完全由
		QTextDocument::find()函数返回的QTextCursor决定。

		\a options参数与QTextDocument::find()函数的flags参数完全相同。

		当\a multiSelection为真时，TextEdit会为每个匹配项创建选区以将它们临时高亮。请注意，
		这个临时选区只是视觉效果，不改变当前光标位置。如果需要清除这些临时选区，请调用clearFindAllSelection()函数。
	*/
	QList<QTextCursor> TextEdit::findAll(const QString& source, bool sourceAsRe, QTextDocument::FindFlags options, bool multiSelection) const {
		QList<QTextCursor> results;
		if (sourceAsRe) {
			QRegularExpression re(source);
			QTextCursor cursor(d->Text->document());
			cursor.setPosition(0);
			while (!cursor.isNull() && !cursor.atEnd()) {
				QTextCursor found = d->Text->document()->find(re, cursor, options);
				if (found.isNull() || found.position() == cursor.position()) {
					break;
				}
				results.append(found);
				cursor = found;
			}
		}
		else {
			QTextCursor cursor(d->Text->document());
			cursor.setPosition(0);
			while (!cursor.isNull() && !cursor.atEnd()) {
				QTextCursor found = d->Text->document()->find(source, cursor, options);
				if (found.isNull() || found.position() == cursor.position()) {
					break;
				}
				results.append(found);
				cursor = found;
			}
		}
		if (multiSelection) {
			d->createFindAllMultiSelection(results);
		}
		return results;
	}
	
	/*!
		\since YSS 0.14.0
		清除findAll函数创建的临时选区。临时选区只是视觉效果，不改变当前光标位置。
	*/
	void TextEdit::clearFindAllSelection() {
		d->clearFindAllMultiSelection();
	}
	
	/*!
		\since YSS 0.14.0
		在整个文档中从\a from 开始，尝试查找下一个\a source。当\a sourceAsRe为真时，将\a source视作正则表达式进行查找。
		返回匹配的QTextCursor。如果没有找到匹配项，则返回一个isNull()为真的QTextCursor。这个QTextCursor的特性完全由
		QTextDocument::find()函数返回的QTextCursor决定。
		如果\a from是-1，则从当前光标位置开始查找。如果\a from 是0，就从文档开头开始查找。
		\a options参数与QTextDocument::find()函数的flags参数完全相同。

		\a relocate 如果为true，就在找到匹配项后将光标移动到该位置；如果为false，就不改变当前光标位置。默认为false
	*/
	QTextCursor TextEdit::findNext(const QString& source, bool sourceAsRe, qint32 from, QTextDocument::FindFlags options, bool relocate) const {
		if (from == -1) {
			from = d->Text->textCursor().position();
		}
		QTextCursor result;
		if (sourceAsRe) {
			QRegularExpression re(source);
			result = d->Text->document()->find(re, from, options);
		}
		else {
			result = d->Text->document()->find(source, from, options);
		}
		if (relocate && !result.isNull()) {
			d->Text->setTextCursor(result);
		}
		return result;
	}

	/*!
		\since YSS 0.14.0
		这函数使用 findAll 找到所有匹配项之后，将它们全部替换为 \a newText。返回替换掉的匹配项数量。
		\a text \a textAsRe \a options参数与findAll函数的对应参数完全相同。
	*/
	qint32 TextEdit::replaceAll(const QString& source, const QString& newText, bool sourceAsRe, QTextDocument::FindFlags options) {
		QList<QTextCursor> matches = findAll(source, sourceAsRe, options);
		QTextCursor opeCusor = d->Text->textCursor();
		opeCusor.beginEditBlock();
		for (QTextCursor& cursor : matches) {
			opeCusor.setPosition(cursor.selectionStart());
			opeCusor.setPosition(cursor.selectionEnd(), QTextCursor::KeepAnchor);
			opeCusor.insertText(newText);
		}
		opeCusor.endEditBlock();
		return matches.size();
	}

	/*!
		\since YSS 0.14.0
		这函数使用 findNext 找到下一个匹配项之后，将它替换为 \a newText。返回是否成功替换。
		\a text 、\a textAsRe 、\a from、\a options、\a relocate 参数与findNext函数的对应参数完全相同。
	*/
	bool TextEdit::replaceNext(const QString& source, const QString& newText, bool sourceAsRe, qint32 from, QTextDocument::FindFlags options, bool relocate) {
		QTextCursor match = findNext(source, sourceAsRe, from, options);
		if (match.isNull()) {
			return false;
		}
		else {
			match.insertText(newText);
			if (relocate) {
				d->Text->setTextCursor(match);
			}
			return true;
		}
	}

	/*!
		\since YSS 0.14.0
		显示查找和替换页面。这个页面会在TextEdit的右上角显示，您也可以通过拖动来调整它的位置。
		如果当前有选中的文本，那么这个选中的文本会被自动填充到查找输入框中。
		请注意，它默认可用快捷键Ctrl+F来打开，您也可以通过调用这个函数来打开它。
	*/
	void TextEdit::showFindAndReplace() {
		d->showFindAndReplace();
	}

	/*!
		\since YSS 0.13.0
		按给定行号和列重定位光标。行号和列号均从0开始。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用cursorToPosition()函数。
	*/
	bool TextEdit::onCursorToPosition(qint32 line, qint32 column) {
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
		\since YSS 0.13.0
		打开一个文件。只有文件完全打开成功才会返回true，其他任何失败情况均返回false。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用openFile()函数。
	*/
	bool TextEdit::onOpen(const QString& path) {
		disconnect(d->Text, &QTextEdit::textChanged, this, &TextEdit::setFileChanged);
		QString ext = QFileInfo(path).suffix();
		YSSCore::Editor::LangServer* server = YSSLSM->routeExt(ext);
		if (server != nullptr) {
			if (d->Highlighter != nullptr) {
				d->Highlighter->setParent(nullptr);
				d->Highlighter->deleteLater();
			}
			d->Highlighter = server->createHighlighter(this);

			if (d->TabCompleter != nullptr) {
				d->TabCompleter->setParent(nullptr);
				d->TabCompleter->deleteLater();
			}
			d->TabCompleter = server->createTabCompleter(this);

			if (d->TabCompleterWidget != nullptr) {
				d->TabCompleterWidget->setParent(nullptr);
				d->TabCompleterWidget->deleteLater();
			}
			if (d->TabCompleter != nullptr) {
				d->TabCompleterWidget = new YSSCore::__Private__::TabCompleterWidget(d->Text);
			}
			d->HoverInfoProvider = server->createHoverInfoProvider(this);
			if (d->HoverInfoWidget != nullptr) {
				d->HoverInfoWidget->setParent(nullptr);
				d->HoverInfoWidget->deleteLater();
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
		d->LastCursor.movePosition(QTextCursor::Start);
		file.close();
		cancelFileChanged();
		connect(d->Text, &QTextEdit::textChanged, [this]() {
			if (not d->Rehighlighting) {
				this->setFileChanged();
			}
			});
		return true;
	}

	/*!
		\since YSS 0.13.0
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
		\since YSS 0.13.0
		保存当前文件。只有当文件成功保存时才会返回true，其他任何失败情况均返回false。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用saveFile()函数。
	*/
	bool TextEdit::onSave(const QString& path) {
		QFile file(path);
		QFileInfo fileInfo(path);
		if (fileInfo.exists() && not fileInfo.isWritable()) {
			QMessageBox::warning(this, 
				VITR("YSS::editor.textEdit.readOnly.title"), VITR("YSS::editor.textEdit.readOnly.message").arg(fileInfo.fileName()),
				QMessageBox::Ok);
			vgWarning << "File is read-only:" << path << ",";
			return false;
		}
		else if (fileInfo.exists() && fileInfo.isWritable()) {
			if (not file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				vgErrorF << "Failed to save file:" << path;
				return false;
			}
			file.write(d->Text->toPlainText().toUtf8());
			file.close();
			return true;
		}
		else if (not fileInfo.exists()) {
			if (not file.open(QIODevice::NewOnly | QIODevice::Text)) {
				vgErrorF << "Failed to save file:" << path;
				return false;
			}
			file.write(d->Text->toPlainText().toUtf8());
			file.close();
			return true;
		}
		else {
			vgErrorF << "Unknown error when saving file:" << path;
			return false;
		}
	}

	/*!
		\since YSS 0.13.0
		重新加载当前文件。只有当文件成功重新加载时才会返回true，其他任何失败情况均返回false。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用reloadFile()函数。
	*/
	bool TextEdit::onReload() {
		if (isFileChanged()) {
			QMessageBox msgBox;
			msgBox.setText("YSS::editor.reloadWarning.message");
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Yes);
			int ret = msgBox.exec();
			switch (ret) {
			case QMessageBox::Yes:
				openFile(getFilePath());
				return true;
				break;
			case QMessageBox::Cancel:
				break;
			default:
				break;
			}
		}
		return false;
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