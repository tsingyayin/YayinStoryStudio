#include <QtCore/qtimer.h>
#include <QtGui/qcursor.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpalette.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qtextedit.h>
#include <General/Log.h>
#include <Widgets/ThemeManager.h>
#include "Editor/HoverInfoProvider.h"
#include "Editor/private/HoverInfoProvider_p.h"
#include "Editor/private/TabCompleterProvider_p.h"
#include "Editor/private/TextEdit_p.h"
#include "Editor/SyntaxHighlighter.h"
#include "Editor/TabCompleterProvider.h"
#include "Editor/TextEdit.h"

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
		// 焦点在内层 d->Text 上，失焦事件由 d->Text 触发而非 TextEdit 本体，
		// 因此放在通用入口处处理，确保编辑器失去焦点时补全提示自动收起。
		if (event->type() == QEvent::FocusOut) {
			if (TabCompleterWidget->isVisible()) {
				TabCompleterWidget->hide();
			}
		}
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
						if (TabCompleterWidget->isVisible()) {
							if (keyEvent->modifiers() == Qt::NoModifier && (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)) {
								onDirectionClicked(keyEvent);
								return true;
							}
							else if (keyEvent->modifiers() == Qt::ControlModifier && (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)) {
								useKeyboardToMoveCursor = true;
								auto cursor = Text->textCursor();
								if (keyEvent->key() == Qt::Key_Up) {
									cursor.movePosition(QTextCursor::Up);
								}
								else {
									cursor.movePosition(QTextCursor::Down);
								}
								Text->setTextCursor(cursor);
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
			else if (event->type() == QEvent::MouseButtonPress) {
				auto mouseEvent = static_cast<QMouseEvent*>(event);
				useKeyboardToMoveCursor = false;
				if (mouseEvent->button() == Qt::LeftButton) {
					if (TabCompleterWidget->isVisible()) {
						TabCompleterWidget->hide();
					}
					if (HoverInfoWidget->isVisible()) {
						HoverInfoWidget->hide();
					}
				}
				return false;
			}
		}
		else if (obj == q) {
			if (event->type() == QEvent::Hide || event->type() == QEvent::HideToParent) {
				if (HoverInfoWidget->isVisible()) {
					HoverInfoWidget->hide();
				}
				if (TabCompleterWidget->isVisible()) {
					TabCompleterWidget->hide();
				}
				return false;
			}
			else if (event->type() == QEvent::Show || event->type() == QEvent::ShowToParent) {
				if (HoverInfoWidget->isVisible()) {
					HoverInfoWidget->hide();
				}
				if (TabCompleterWidget->isVisible()) {
					TabCompleterWidget->hide();
				}
				return false;
			}
			else if (event->type() == QEvent::Resize) {
				FindAndReplaceWidget->move(q->width() - 352, 0); // 340 for width, 12 for vertical scrollbar width
				adjustTabCompleterPosition();
				adjustHoverInfoPosition(Text->textCursor());
			}
		}
		else if (obj == Line->viewport()) {
			if (event->type() == QEvent::Wheel) { 
				if ((static_cast<QWheelEvent*>(event))->modifiers() & Qt::ControlModifier) {
					return true; // forbid zooming in/out when Ctrl + mouse wheel
				}
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
		
		QTextCursor cursor = Text->textCursor();
		int index = cursor.block().blockNumber();
		int column = cursor.positionInBlock();
		int selection = cursor.selectedText().length();
		if (index >= Line->document()->blockCount()) {
			onBlockCountChanged(index + 1);
		}
		if (useKeyboardToMoveCursor) {
			if (TabCompleterWidget->isVisible()) {
				TabCompleterWidget->hide();
			}
			if (HoverInfoWidget->isVisible()) {
				HoverInfoWidget->hide();
			}
			useKeyboardToMoveCursor = false;
			emit q->cursorPositionChanged(cursor);
			return;
		}
		if (selection != 0) {
			if (TabCompleterWidget->isVisible()) {
				TabCompleterWidget->hide();
			}
			if (HoverInfoWidget->isVisible()) {
				HoverInfoWidget->hide();
			}
			CurrentLineSelection.format.setBackground(Qt::transparent);
			onMultiSelectionChanged();
			emit q->cursorPositionChanged(cursor);
			return;
		}
		onCompleter();
		onHoverInfo(false);
		CurrentLineSelection.cursor = cursor;
		CurrentLineSelection.cursor.clearSelection();
		CurrentLineSelection.format.setBackground(Text->palette().color(QPalette::AlternateBase).lighter(100));
		CurrentLineSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
		onMultiSelectionChanged();
		emit q->cursorPositionChanged(cursor);
	}

	void TextEditPrivate::onCompleter() {
		QTextCursor cursor = Text->textCursor();
		if (cursor.hasSelection()) {
			return;
		}
		if (Text->isReadOnly()) {
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
			if (CompleterLevel == YSSCore::Editor::TabCompleterItem::CompleterLevel::None) {
				TabCompleterWidget->hide();
				return;
			}
			QList<YSSCore::Editor::TabCompleterItem> filteredList;
			QList<YSSCore::Editor::TabCompleterItem> top;
			QList<YSSCore::Editor::TabCompleterItem> high;
			QList<YSSCore::Editor::TabCompleterItem> medium;
			QList<YSSCore::Editor::TabCompleterItem> low;
			for (auto& item : list) {
				if (item.getCompleterLevel() <= CompleterLevel) {
					switch (item.getCompleterPriority()) {
					case YSSCore::Editor::TabCompleterItem::CompleterPriority::Top:
						item.setText("★ " + item.getText());
						top.append(item);
						break;
					case YSSCore::Editor::TabCompleterItem::CompleterPriority::High:
						high.append(item);
						break;
					case YSSCore::Editor::TabCompleterItem::CompleterPriority::Medium:
						medium.append(item);
						break;
					case YSSCore::Editor::TabCompleterItem::CompleterPriority::Low:
						low.append(item);
						break;
					}
				}
			}
			filteredList.append(top);
			filteredList.append(high);
			filteredList.append(medium);
			filteredList.append(low);
			///yDebugF << "TabCompleterWidget is shown";
			TabCompleterWidget->setCompleterItems(filteredList);
			adjustTabCompleterPosition();
			TabCompleterWidget->show();
			Text->setFocus();
		}
	}

	void TextEditPrivate::onTabClicked(QKeyEvent* event) {
		if (TabCompleterWidget->isVisible()) {
			if (event->key() == Qt::Key_Tab) {
				onTabClicked_TabCompleter(event);
			}
			else {
				TabCompleterWidget->hide();
			}
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
		if (TabCompleterWidget->isVisible()) {
			TabCompleterWidget->hide();
		}
		if (HoverInfoWidget->isVisible()) {
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
			if (event->key() == Qt::Key_Up) {
				TabCompleterWidget->selectPrevious();
			}
			else {
				TabCompleterWidget->selectNext();
			}
		}
	}

	// 判定"鼠标靠近目标控件"时，向控件外扩的像素数，nearArea 与 moveVectorPointTo 共用。
	// 两个函数的光标坐标都是全局坐标。
	constexpr int ApproachingBorder = 20;

	static bool nearArea(QWidget* target, QPointF mousePos, int border = ApproachingBorder) {
		QRect area = target->geometry();
		area.adjust(-border, -border, border, border);
		QPoint pos = target->parentWidget()->mapFromGlobal(mousePos.toPoint());
		return area.contains(pos);
	}

	// whether the mouse is trying to move close to the target widget, if so, return true, otherwise return false.
	// if nearArea ,directly return true.
	// startPos 与 endPos 都是全局坐标，startPos 是上一次采样的光标位置，endPos 是当前光标位置。
	static bool moveVectorPointTo(QPointF startPos, QPointF endPos, QWidget* target) {
		if (nearArea(target, endPos)) {
			return true;
		}
		QPointF moveVector = endPos - startPos;
		if (qFuzzyIsNull(moveVector.x()) and qFuzzyIsNull(moveVector.y())) {
			return false;
		}
		// 目标控件在全局坐标下的范围，与 nearArea 一样外扩一圈
		QRectF area(target->mapToGlobal(QPoint(0, 0)), QSizeF(target->size()));
		area.adjust(-ApproachingBorder, -ApproachingBorder, ApproachingBorder, ApproachingBorder);
		// 光标到该范围的最近点：把光标坐标分别钳进水平、垂直范围即可
		QPointF nearestTarget(
			qBound(area.left(), startPos.x(), area.right()),
			qBound(area.top(), startPos.y(), area.bottom()));
		// 移动方向与"光标指向最近点"的方向夹角小于 90 度，即认为鼠标正在朝目标靠过来。
		// 区域是凸的，沿这个方向移动距离必然减小，不会出现擦边平移时误判的情况。
		return QPointF::dotProduct(nearestTarget - startPos, moveVector) > 0.0;
	}

	void TextEditPrivate::onMouseMove(QMouseEvent* event) {
		QPoint currentPos = event->globalPosition().toPoint();
		float dist = std::sqrt(std::pow(LastMousePos.x() - currentPos.x(), 2) + std::pow(LastMousePos.y() - currentPos.y(), 2));
		if (dist <= qApp->startDragDistance()) {
			return;
		}
		QPoint lastPos = LastMousePos;
		LastMousePos = currentPos;
		if (not HoverInfoWidget->isVisible()) {
			HoverTimer->start(HoverTimeout);
			return;
		}
		if (moveVectorPointTo(lastPos, currentPos, HoverInfoWidget)) {
			return;
		}
		HoverInfoWidget->hide();
	}

	

	bool TextEditPrivate::onMouseScroll(QWheelEvent* event) {
		if (HoverTimer->isActive()) {
			HoverTimer->stop();
		}
		if (Text->isReadOnly() && (event->modifiers() & Qt::ControlModifier)) {	
			return true;
		}
		if (HoverInfoWidget->isVisible()) {
			if (nearArea(HoverInfoWidget, event->globalPosition())) {
				HoverInfoWidget->scrollBy(-event->angleDelta().y());
				return true;
			}
			else {
				HoverInfoWidget->hide();
				return false;
			}
		}
		else if (TabCompleterWidget->isVisible()) {
			if (nearArea(TabCompleterWidget, event->globalPosition())) {
				TabCompleterWidget->scrollBy(-event->angleDelta().y());
				return true;
			}
			else {
				TabCompleterWidget->hide();
				return false;
			}
		}
		else {
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
		if (Text->isReadOnly()) {
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
				adjustHoverInfoPosition(cursor);
				// 记录提示弹出时的光标位置：之后鼠标是否"正在朝提示移动"就以它为起点判定
				LastMousePos = QCursor::pos();
				HoverInfoWidget->show();
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
		adjustTabCompleterPosition();
		adjustHoverInfoPosition(Text->textCursor());
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
			onMultiSelectionChanged();
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
			onMultiSelectionChanged();
		}
	}

	void TextEditPrivate::clearAltMultiSelection() {
		if (!AltMultiSelections.isEmpty()) {
			AltMultiSelections.clear();
			onMultiSelectionChanged();
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
		onMultiSelectionChanged();
	}

	void TextEditPrivate::clearFindAllMultiSelection() {
		if (not FindAllMultiSelections.isEmpty()) {
			FindAllMultiSelections.clear();
			onMultiSelectionChanged();
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

	void TextEditPrivate::onMultiSelectionChanged() {
		QList<QTextEdit::ExtraSelection> allSelections = AltMultiSelections + FindAllMultiSelections;
		allSelections << CurrentLineSelection;
		Text->setExtraSelections(allSelections);
	}

	void TextEditPrivate::adjustTabCompleterPosition() {
		TabCompleterWidget->adjustHeight(TabCompleterWidget->getMaxAllowedHeight());
		QRect pos = Text->cursorRect();
		if (not HoverArea) {
			bool rightOut = pos.x() + TabCompleterWidget->width() > Text->viewport()->width();
			if (rightOut) {
				//right align
				TabCompleterWidget->move(pos.x() + 10 - TabCompleterWidget->width(), pos.y() + 20);
			}
			else {
				// left align
				TabCompleterWidget->move(pos.x() + 10, pos.y() + 20);
			}
			// 垂直方向：下侧剩余高度（Text 视口坐标系）
			qint32 downY = pos.y() + 20;
			qint32 remainingBelow = Text->viewport()->height() - downY;
			if (TabCompleterWidget->height() > remainingBelow) {
				// 下超边界：先将剩余高度设置给 TabCompleterWidget（内部会进行钳制）
				TabCompleterWidget->adjustHeight(remainingBelow);
				if (TabCompleterWidget->height() < remainingBelow) {
					// 钳制后能放入下侧剩余空间，保持顶端对齐即可
					return;
				}
				// 钳制后仍超出：改为底部对齐（向上移），同样先设置剩余高度以进行钳制；若仍超出上边界则忽略
				TabCompleterWidget->adjustHeight(pos.y() - 10);
				TabCompleterWidget->move(TabCompleterWidget->x(), pos.y() - 10 - TabCompleterWidget->height());
			}
			else {
				// 不超界：顶端对齐
				TabCompleterWidget->move(pos.x() + 10, downY);
			}
		}
		else {
			auto hpos = Text->mapTo(HoverArea, pos.topLeft());
			bool rightOut = hpos.x() + TabCompleterWidget->width() > HoverArea->width();
			if (rightOut) {
				//right align
				TabCompleterWidget->move(Text->mapTo(HoverArea, QPoint(pos.x() + 10 - TabCompleterWidget->width(), pos.y() + 20)));
			}
			else {
				// left align
				TabCompleterWidget->move(Text->mapTo(HoverArea, QPoint(pos.x() + 10, pos.y() + 20)));
			}
			// 垂直方向：下侧剩余高度（HoverArea 坐标系）
			qint32 downY = hpos.y() + 20;
			qint32 remainingBelow = HoverArea->height() - downY;
			if (TabCompleterWidget->height() > remainingBelow) {
				// 下超边界：先将剩余高度设置给 TabCompleterWidget（内部会进行钳制）
				TabCompleterWidget->adjustHeight(remainingBelow);
				if (TabCompleterWidget->height() < remainingBelow) {
					// 钳制后能放入下侧剩余空间，保持顶端对齐即可
					return;
				}
				// 钳制后仍超出：改为底部对齐（向上移），同样先设置剩余高度以进行钳制；若仍超出上边界则忽略
				TabCompleterWidget->adjustHeight(hpos.y() - 10);
				TabCompleterWidget->move(TabCompleterWidget->x(), hpos.y() - 10 - TabCompleterWidget->height());
			}
			else {
				// 不超界：顶端对齐
				TabCompleterWidget->move(TabCompleterWidget->x(), downY);
			}
		}
	}

	void TextEditPrivate::adjustHoverInfoPosition(const QTextCursor& cursor) {
		if (not HoverInfoProvider) {
			return;
		}
		if (TabCompleterWidget->isVisible()) {
			QRect tpos = TabCompleterWidget->geometry();
			// 同时计算 TabCompleterWidget 左右两侧的间隙，选择较大的那个用于显示 HoverInfo
			qint32 containerWidth = HoverArea ? HoverArea->width() : Text->viewport()->width();
			qint32 leftGap = tpos.x();
			qint32 rightGap = containerWidth - (tpos.x() + tpos.width());
			qint32 largerGap = qMax(leftGap, rightGap);
			if (largerGap < HoverInfoWidget::DefaultWidth) {
				// 较大间隙也放不下完整宽度的 HoverInfo：调整宽度到恰好能显示整个 HoverInfo
				HoverInfoWidget->setFixedWidth(qMax(0, largerGap));
			}
			else {
				// 间隙足够：恢复正常宽度
				HoverInfoWidget->recoverDefaultWidth();
			}
			if (rightGap >= leftGap) {
				// 右侧间隙较大：靠 TabCompleterWidget 右边缘显示
				HoverInfoWidget->move(tpos.x() + tpos.width(), tpos.y());
			}
			else {
				// 左侧间隙较大：靠 TabCompleterWidget 左边缘显示
				HoverInfoWidget->move(tpos.x() - HoverInfoWidget->width(), tpos.y());
			}
		}
		else {
			HoverInfoWidget->recoverDefaultWidth();
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
}
