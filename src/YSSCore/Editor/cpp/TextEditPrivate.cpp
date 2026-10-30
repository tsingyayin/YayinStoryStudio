#include <QtCore/qset.h>
#include <QtCore/qtimer.h>
#include <QtGui/qcursor.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpalette.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qtextedit.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <Widgets/ThemeManager.h>
#include "Editor/HoverInfoProvider.h"
#include "Editor/InlayHint.h"
#include "Editor/InlayHintHoverWidget.h"
#include "Editor/InlayHintManager.h"
#include "Editor/InlayHintProvider.h"
#include "Editor/VirtualFilePath.h"
#include "Editor/private/HoverInfoProvider_p.h"
#include "Editor/private/InlayHint_p.h"
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
			// 焦点进入图片提示浮窗内部的输入控件时不能收起浮窗，否则浮窗里的交互控件无法使用。
			if (not isFocusInInlayHintHoverWidget()) {
				hideInlayHintHover();
			}
			if (SgtKind != SuggestionKind::None) {
				dismissSuggestion(true);
			}
		}
		// 输入法组字意味着用户开始输入别的内容，同样按放弃处理。
		if (event->type() == QEvent::InputMethod and SgtKind != SuggestionKind::None) {
			dismissSuggestion(true);
		}
		if (obj == Text) {
			if (event->type() == QEvent::KeyPress) {
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
				// 待抉择建议自己的按键：Tab/Enter 接受，Esc 拒绝，方向键切换，输入别的字符视为放弃。
				// 自动补全可见时 Tab 与方向键归补全，避免两套交互抢同一个键。
				if (SgtKind != SuggestionKind::None and not TabCompleterWidget->isVisible()) {
					const int key = keyEvent->key();
					if (key == Qt::Key_Tab) {
						acceptSuggestion();
						return true;
					}
					else if (key == Qt::Key_Return or key == Qt::Key_Enter) {
						if (not Text->textCursor().hasSelection()) {
							acceptSuggestion();
							return true;
						}
					}
					else if (key == Qt::Key_Escape) {
						dismissSuggestion(true);
						return true;
					}
					else if ((key == Qt::Key_Up or key == Qt::Key_Left) and keyEvent->modifiers() == Qt::NoModifier) {
						cycleSuggestion(-1);
						return true;
					}
					else if ((key == Qt::Key_Down or key == Qt::Key_Right) and keyEvent->modifiers() == Qt::NoModifier) {
						cycleSuggestion(1);
						return true;
					}
					else if (not keyEvent->text().isEmpty() and keyEvent->text().at(0).isPrint()) {
						dismissSuggestion(true);
					}
				}
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
			if (event->type() == QEvent::Resize) {
				positionTestButton();
				// 内置水平滚动条出现/消失时视口高度会变化；行号栏和缩略图只占视口高度，
				// 避免在滚动条所在的高度上继续画行号或缩略图。
				const qint32 viewportHeight = Text->viewport()->height();
				if (viewportHeight > 0) {
					if (Line != nullptr) {
						Line->setFixedHeight(viewportHeight);
					}
					if (Overview != nullptr) {
						Overview->setFixedHeight(viewportHeight);
					}
				}
			}
			if (event->type() == QEvent::MouseMove) {
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				onMouseMove(mouseEvent);
				return false;
			}
			else if (event->type() == QEvent::Leave) {
				YSSCore::Editor::InlayHintHoverWidget* visibleWidget =
					InlayHintHoverWidgets.value(InlayHintHoverCurrentID, nullptr);
				if (visibleWidget != nullptr and visibleWidget->isVisible()) {
					QWidget* underMouse = QApplication::widgetAt(QCursor::pos());
					if (underMouse != visibleWidget and (underMouse == nullptr or not visibleWidget->isAncestorOf(underMouse))) {
						hideInlayHintHover();
					}
				}
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
					qint32 hitLine = -1;
					qint32 hitColumn = -1;
					QString hitMulti;
					if (findInlayHintAt(mouseEvent->position().toPoint(), hitLine, hitColumn, hitMulti)) {
						InlayHintHoverTimer->stop();
						showInlayHintHover(hitLine, hitColumn, hitMulti);
					}
					else {
						hideInlayHintHover();
					}
					if (SgtKind != SuggestionKind::None) {
						// 点击正文意味着用户放弃了当前建议，但点击本身照常生效。
						dismissSuggestion(true);
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
				hideInlayHintHover();
				return false;
			}
			else if (event->type() == QEvent::Show || event->type() == QEvent::ShowToParent) {
				if (HoverInfoWidget->isVisible()) {
					HoverInfoWidget->hide();
				}
				if (TabCompleterWidget->isVisible()) {
					TabCompleterWidget->hide();
				}
				hideInlayHintHover();
				return false;
			}
			else if (event->type() == QEvent::Resize) {
				FindAndReplaceWidget->move(q->width() - 352, 0); // 340 for width, 12 for vertical scrollbar width
				hideInlayHintHover();
				adjustTabCompleterPosition();
				adjustHoverInfoPosition(Text->textCursor());
				adjustSuggestionPosition();
			}
		}
		else if (obj == Line) {
			if (event->type() == QEvent::Wheel) { 
				if ((static_cast<QWheelEvent*>(event))->modifiers() & Qt::ControlModifier) {
					return true; // forbid zooming in/out when Ctrl + mouse wheel
				}
			}
		}
		return false;
	}

	void TextEditPrivate::onBlockCountChanged(qint32 count) {
		if (count == LineCount) {
			return;
		}
		LineCount = count;
		hideInlayHintHover();
		// 行号现在是画出来的，这里只更新位数对应的宽度，不再逐行维护文本块。
		Line->updateLineCount(count);
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
		if (index >= Text->document()->blockCount()) {
			onBlockCountChanged(index + 1);
		}
		if (useKeyboardToMoveCursor) {
			if (TabCompleterWidget->isVisible()) {
				TabCompleterWidget->hide();
			}
			if (HoverInfoWidget->isVisible()) {
				HoverInfoWidget->hide();
			}
			hideInlayHintHover();
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
			hideInlayHintHover();
			CurrentLineSelection.format.setBackground(Qt::transparent);
			onMultiSelectionChanged();
			emit q->cursorPositionChanged(cursor);
			return;
		}
		onCompleter();
		// 补全触发之后统一检查一次：文档里已经没有人使用的图片提示浮窗在这里销毁。
		syncInlayHintHoverWidgets();
		onHoverInfo(false);
		CurrentLineSelection.cursor = cursor;
		CurrentLineSelection.cursor.clearSelection();
		CurrentLineSelection.format.setBackground(Text->palette().color(QPalette::AlternateBase).lighter(100));
		CurrentLineSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
		onMultiSelectionChanged();
		emit q->cursorPositionChanged(cursor);
	}

	void TextEditPrivate::onCompleter() {
		// 待抉择状态独占浮层，补全不能在此期间冒出来抢键。
		if (SgtKind != SuggestionKind::None) {
			return;
		}
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
			hideInlayHintHover();
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
		hideInlayHintHover();
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
		QPoint lastPos = LastMousePos;
		const QPoint viewportPos = Text->viewport()->mapFromGlobal(currentPos);

		// 图片提示区域优先：悬停到它上面时不再触发普通的文本悬停信息。
		qint32 hitLine = -1;
		qint32 hitColumn = -1;
		QString hitMulti;
		if (findInlayHintAt(viewportPos, hitLine, hitColumn, hitMulti)) {
			YSSCore::Editor::VirtualFilePath hitPath(hitMulti);
			const QString hitProviderID = hitPath.getExt();
			ensureInlayHintHoverWidget(hitProviderID);
			if (InlayHintHoverWidgets.value(hitProviderID, nullptr) != nullptr) {
				if (HoverInfoWidget->isVisible()) {
					HoverInfoWidget->hide();
				}
				HoverTimer->stop();
				YSSCore::Editor::InlayHintHoverWidget* visibleWidget =
					InlayHintHoverWidgets.value(InlayHintHoverCurrentID, nullptr);
				if (visibleWidget != nullptr and visibleWidget->isVisible()) {
					if ((hitLine == visibleWidget->getBlockNumber() and hitColumn == visibleWidget->getColumn())
						or nearArea(visibleWidget, event->globalPosition())) {
						LastMousePos = currentPos;
						return;
					}
					hideInlayHintHover();
				}
				if (InlayHintHoverTimer->isActive() and InlayHintHoverPendingLine == hitLine
					and InlayHintHoverPendingColumn == hitColumn and InlayHintHoverPendingMulti == hitMulti) {
					LastMousePos = currentPos;
					return;
				}
				InlayHintHoverPendingLine = hitLine;
				InlayHintHoverPendingColumn = hitColumn;
				InlayHintHoverPendingMulti = hitMulti;
				InlayHintHoverTimer->start(HoverTimeout);
				LastMousePos = currentPos;
				return;
			}
		}

		if (InlayHintHoverTimer->isActive()) {
			InlayHintHoverTimer->stop();
			InlayHintHoverPendingLine = -1;
			InlayHintHoverPendingColumn = -1;
			InlayHintHoverPendingMulti.clear();
		}
		YSSCore::Editor::InlayHintHoverWidget* visibleWidget =
			InlayHintHoverWidgets.value(InlayHintHoverCurrentID, nullptr);
		if (visibleWidget != nullptr and visibleWidget->isVisible()) {
			if (moveVectorPointTo(lastPos, currentPos, visibleWidget)
				or nearArea(visibleWidget, event->globalPosition())) {
				LastMousePos = currentPos;
				return;
			}
			hideInlayHintHover();
		}

		float dist = std::sqrt(std::pow(lastPos.x() - currentPos.x(), 2) + std::pow(lastPos.y() - currentPos.y(), 2));
		if (dist <= qApp->startDragDistance()) {
			return;
		}
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
		if (InlayHintHoverTimer->isActive()) {
			InlayHintHoverTimer->stop();
		}
		hideInlayHintHover();
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
		// 待抉择状态独占浮层，悬停信息也不能在此期间冒出来。
		if (SgtKind != SuggestionKind::None) {
			return;
		}
		YSSCore::Editor::InlayHintHoverWidget* inlayWidget =
			InlayHintHoverWidgets.value(InlayHintHoverCurrentID, nullptr);
		if (inlayWidget != nullptr and inlayWidget->isVisible()) {
			return;
		}
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

	bool TextEditPrivate::findInlayHintAt(const QPoint& viewportPos, qint32& lineNumber, qint32& column, QString& multi) const {
		if (Text == nullptr) {
			return false;
		}
		return Text->getInlayHintAt(viewportPos, lineNumber, column, multi);
	}

	void TextEditPrivate::ensureInlayHintHoverWidget(const QString& providerID) {
		if (InlayHintHoverWidgets.contains(providerID)) {
			return;
		}
		YSSCore::Editor::InlayHintProvider* provider = YSSIHM->getProvider(providerID);
		if (provider == nullptr) {
			// 记一个空项，避免每次鼠标经过都重新查询同一个没有浮窗的 Provider。
			InlayHintHoverWidgets.insert(providerID, nullptr);
			return;
		}
		YSSCore::Editor::InlayHintHoverWidget* widget = provider->createHoverWidget(q->getUri());
		if (widget == nullptr) {
			InlayHintHoverWidgets.insert(providerID, nullptr);
			return;
		}
		widget->setParent(HoverArea ? HoverArea : Text);
		widget->hide();
		connect(widget, &YSSCore::Editor::InlayHintHoverWidget::requestModify, this,
			[this, widget](qint32 startColumn, qint32 rawLength, QString& newContent) {
				const qint32 blockNumber = widget->getBlockNumber();
				QTextBlock block = Text->document()->findBlockByNumber(blockNumber);
				if (not block.isValid()) {
					return;
				}
				const QString rawText = block.text();
				if (startColumn < 0 or rawLength < 0 or startColumn + rawLength > rawText.length()) {
					return;
				}
				QTextCursor cursor(block);
				cursor.setPosition(block.position() + startColumn);
				cursor.setPosition(block.position() + startColumn + rawLength, QTextCursor::KeepAnchor);
				cursor.insertText(newContent);
				Text->setTextCursor(cursor);
				hideInlayHintHover();
			});
		connect(widget, &YSSCore::Editor::InlayHintHoverWidget::requestClose, this, [this]() {
			hideInlayHintHover();
			});
		connect(widget, &QObject::destroyed, this, [this, providerID, widget]() {
			if (InlayHintHoverWidgets.value(providerID, nullptr) == widget) {
				InlayHintHoverWidgets.remove(providerID);
			}
			if (InlayHintHoverCurrentID == providerID) {
				InlayHintHoverCurrentID.clear();
			}
			});
		InlayHintHoverWidgets.insert(providerID, widget);
	}

	void TextEditPrivate::syncInlayHintHoverWidgets() {
		if (not InlayHintHoverDirty) {
			return;
		}
		InlayHintHoverDirty = false;
		QSet<QString> usedProviders;
		if (Highlighter != nullptr) {
			const QMap<qint32, QList<YSSCore::Editor::InlayHint>> allHints = Highlighter->getAllInlayHints();
			for (auto it = allHints.begin(); it != allHints.end(); ++it) {
				for (const YSSCore::Editor::InlayHint& hint : it.value()) {
					QString providerID;
					if (YSSCore::__Private__::inlayHintUsesProvider(hint, &providerID)) {
						usedProviders.insert(providerID);
					}
				}
			}
		}
		for (const QString& providerID : usedProviders) {
			ensureInlayHintHoverWidget(providerID);
		}
		for (auto it = InlayHintHoverWidgets.begin(); it != InlayHintHoverWidgets.end();) {
			if (usedProviders.contains(it.key())) {
				++it;
				continue;
			}
			if (it.value() != nullptr) {
				if (it.value()->isVisible()) {
					it.value()->hide();
				}
				it.value()->setParent(nullptr);
				it.value()->deleteLater();
			}
			it = InlayHintHoverWidgets.erase(it);
		}
		if (not InlayHintHoverCurrentID.isEmpty() and not InlayHintHoverWidgets.contains(InlayHintHoverCurrentID)) {
			hideInlayHintHover();
		}
	}

	void TextEditPrivate::showInlayHintHover(qint32 lineNumber, qint32 column, const QString& multi) {
		YSSCore::Editor::VirtualFilePath path(multi);
		if (not path.isValid()) {
			hideInlayHintHover();
			return;
		}
		const QString providerID = path.getExt();
		ensureInlayHintHoverWidget(providerID);
		YSSCore::Editor::InlayHintHoverWidget* widget = InlayHintHoverWidgets.value(providerID, nullptr);
		if (widget == nullptr) {
			hideInlayHintHover();
			return;
		}
		QTextBlock block = Text->document()->findBlockByNumber(lineNumber);
		if (not block.isValid()) {
			hideInlayHintHover();
			return;
		}
		for (YSSCore::Editor::InlayHintHoverWidget* other : InlayHintHoverWidgets) {
			if (other != nullptr and other != widget and other->isVisible()) {
				other->hide();
			}
		}
		widget->setTargetFile(q->getUri());
		widget->setBlockNumber(lineNumber);
		widget->setColumn(column);
		widget->setRawText(block.text());
		widget->setPara(path.getParams());
		widget->onHoverRequest();
		widget->adjustSize();
		adjustInlayHintHoverPosition(widget, lineNumber, column);
		InlayHintHoverCurrentID = providerID;
		LastMousePos = QCursor::pos();
		widget->show();
		widget->raise();
	}

	void TextEditPrivate::hideInlayHintHover() {
		if (InlayHintHoverTimer != nullptr) {
			InlayHintHoverTimer->stop();
		}
		// 浮窗内的输入控件拿到焦点时，收起浮窗要把焦点还给编辑器，否则用户接下来无法继续输入。
		const bool returnFocus = isFocusInInlayHintHoverWidget();
		for (YSSCore::Editor::InlayHintHoverWidget* widget : InlayHintHoverWidgets) {
			if (widget != nullptr and widget->isVisible()) {
				widget->hide();
			}
		}
		InlayHintHoverCurrentID.clear();
		InlayHintHoverPendingMulti.clear();
		InlayHintHoverPendingLine = -1;
		InlayHintHoverPendingColumn = -1;
		if (returnFocus and Text != nullptr) {
			Text->setFocus(Qt::OtherFocusReason);
		}
	}

	bool TextEditPrivate::isFocusInInlayHintHoverWidget() const {
		const QWidget* focusWidget = QApplication::focusWidget();
		if (focusWidget == nullptr) {
			return false;
		}
		for (YSSCore::Editor::InlayHintHoverWidget* widget : InlayHintHoverWidgets) {
			if (widget != nullptr and widget->isVisible()
				and (widget == focusWidget or widget->isAncestorOf(focusWidget))) {
				return true;
			}
		}
		return false;
	}

	void TextEditPrivate::adjustInlayHintHoverPosition(QWidget* hoverWidget, qint32 lineNumber, qint32 column) {
		if (hoverWidget == nullptr) {
			return;
		}
		QTextBlock block = Text->document()->findBlockByNumber(lineNumber);
		if (not block.isValid()) {
			return;
		}
		QTextCursor cursor(block);
		cursor.setPosition(block.position() + qBound(0, column, qMax(0, block.length() - 1)));
		const QRect anchorRect = Text->cursorRect(cursor);
		QWidget* container = HoverArea ? HoverArea : Text;
		const QPoint anchor = Text->viewport()->mapTo(container, anchorRect.topLeft());
		const QSize containerSize = HoverArea ? HoverArea->size() : Text->viewport()->size();
		const QSize widgetSize = hoverWidget->size();
		qint32 x = anchor.x();
		qint32 y = anchor.y() + anchorRect.height() + 6;
		if (x + widgetSize.width() > containerSize.width()) {
			x = containerSize.width() - widgetSize.width();
		}
		if (x < 0) {
			x = 0;
		}
		if (y + widgetSize.height() > containerSize.height()) {
			y = anchor.y() - widgetSize.height() - 6;
		}
		if (y < 0) {
			y = 0;
		}
		hoverWidget->move(x, y);
	}

	void TextEditPrivate::clearInlayHintHoverWidgets() {
		hideInlayHintHover();
		InlayHintHoverDirty = true;
		const QList<YSSCore::Editor::InlayHintHoverWidget*> widgets = InlayHintHoverWidgets.values();
		InlayHintHoverWidgets.clear();
		for (YSSCore::Editor::InlayHintHoverWidget* widget : widgets) {
			if (widget != nullptr) {
				widget->setParent(nullptr);
				delete widget;
			}
		}
	}

	void TextEditPrivate::onInlayHintHoverTimeout() {
		if (InlayHintHoverPendingLine < 0 or InlayHintHoverPendingMulti.isEmpty()) {
			return;
		}
		const QPoint viewportPos = Text->viewport()->mapFromGlobal(QCursor::pos());
		qint32 lineNumber = -1;
		qint32 column = -1;
		QString multi;
		if (not findInlayHintAt(viewportPos, lineNumber, column, multi)) {
			return;
		}
		if (lineNumber != InlayHintHoverPendingLine or column != InlayHintHoverPendingColumn
			or multi != InlayHintHoverPendingMulti) {
			return;
		}
		showInlayHintHover(lineNumber, column, multi);
	}

	void TextEditPrivate::onScrollBarChanged(int value) {
		Q_UNUSED(value);
		hideInlayHintHover();
		// 行号栏不再有自己的滚动条，跟随文档滚动重绘可见的那几行
		Line->update();
		adjustTabCompleterPosition();
		adjustHoverInfoPosition(Text->textCursor());
		adjustSuggestionPosition();
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

	/*!
		\a kind 建议类型。行内建议作用于目标行（pureAdd 时追加到行尾，否则替换整行），
		多行建议会在目标行之前插入若干行。
		\a line 目标行号。行内建议是它要作用的行，多行建议是插入位置所在的行（在这些行之前插入）。
		\a items 建议列表，每一项是一条建议的若干行文字。
		\a pureAdd 仅对行内建议有效：建议文字追加到行尾，而不是替换整行。

		进入待抉择状态：在文本右侧显示建议浮窗，多行建议还会在插入位置画出指示线，pureAdd 行内建议会把文字画在行尾。
		期间 Tab/Enter 接受、Esc 拒绝、方向键切换建议、输入其他字符或点击正文则放弃。
		进入时会收起自动补全和悬停信息浮窗，并在整个待抉择期间阻止它们再次出现，避免几套浮层互相抢键。
	*/
	void TextEditPrivate::showSuggestion(SuggestionKind kind, qint32 line, const QList<QStringList>& items, bool pureAdd) {
		if (kind == SuggestionKind::None or kind == SuggestionKind::Remove or items.isEmpty() or SuggestionWidget == nullptr) {
			dismissSuggestion(false);
			return;
		}
		const qint32 blockCount = Text->document()->blockCount();
		if (line < 0 or (kind == SuggestionKind::Inline ? line >= blockCount : line > blockCount)) {
			dismissSuggestion(false);
			return;
		}
		// 待抉择状态下不接受其他浮层干扰：补全和悬停信息会让用户搞不清现在该按哪个键。
		if (TabCompleterWidget and TabCompleterWidget->isVisible()) {
			TabCompleterWidget->hide();
		}
		if (HoverInfoWidget and HoverInfoWidget->isVisible()) {
			HoverInfoWidget->hide();
		}
		hideInlayHintHover();
		SgtKind = kind;
		SgtLine = line;
		SgtItems = items;
		SgtIndex = 0;
		SgtPureAdd = kind == SuggestionKind::Inline and pureAdd;
		SgtRemoveSize = 0;
		SgtTitleKey.clear();
		updateSuggestionWidget();
		adjustSuggestionPosition();
		SuggestionWidget->show();
		// 指示线、行尾虚影和删除底色都画在文本流里，需要重绘一次。
		Text->viewport()->update();
	}

	/*!
		\a from 要删除的第一行（从 0 开始）。
		\a size 要删除的行数。

		进入行删除的待抉择状态：在要删除的行上叠一层半透明红底，浮窗只显示标题和接受/拒绝。
		行号越界会被钳制到文档范围内。
	*/
	void TextEditPrivate::showRemoveSuggestion(qint32 from, qint32 size) {
		if (SuggestionWidget == nullptr) {
			return;
		}
		const qint32 blockCount = Text->document()->blockCount();
		if (from < 0 or size <= 0 or from >= blockCount) {
			dismissSuggestion(false);
			return;
		}
		if (TabCompleterWidget and TabCompleterWidget->isVisible()) {
			TabCompleterWidget->hide();
		}
		if (HoverInfoWidget and HoverInfoWidget->isVisible()) {
			HoverInfoWidget->hide();
		}
		SgtKind = SuggestionKind::Remove;
		SgtLine = from;
		SgtItems.clear();
		SgtIndex = 0;
		SgtPureAdd = false;
		SgtRemoveSize = qMin(size, blockCount - from);
		SgtTitleKey.clear();
		updateSuggestionWidget();
		adjustSuggestionPosition();
		SuggestionWidget->show();
		Text->viewport()->update();
	}

	/*!
		\a notifyRejected 是否发出被拒绝的信号。

		退出待抉择状态。用户放弃（Esc、输入其他字符、点击正文）时应当发信号，
		调用方自己收起建议时则不需要。
	*/
	void TextEditPrivate::dismissSuggestion(bool notifyRejected) {
		if (SgtKind == SuggestionKind::None) {
			return;
		}
		const SuggestionKind kind = SgtKind;
		const qint32 line = SgtLine;
		const qint32 removeSize = SgtRemoveSize;
		const QStringList current = (SgtIndex >= 0 and SgtIndex < SgtItems.size()) ? SgtItems.at(SgtIndex) : QStringList();
		SgtKind = SuggestionKind::None;
		SgtLine = -1;
		SgtItems.clear();
		SgtIndex = 0;
		SgtRemoveSize = 0;
		SgtPureAdd = false;
		SgtTitleKey.clear();
		if (SuggestionWidget) {
			SuggestionWidget->hide();
		}
		Text->viewport()->update();
		if (not notifyRejected) {
			return;
		}
		if (kind == SuggestionKind::Inline) {
			emit q->modifySuggestionRejected(line, current.value(0));
		}
		else if (kind == SuggestionKind::Block) {
			emit q->ghostTextRejected(line, current);
		}
		else {
			emit q->blockRemoveRejected(line, removeSize);
		}
	}

	/*!
		接受当前建议并退出待抉择状态。行内建议替换目标行或追加到行尾，多行建议在目标行之前插入这些行，
		行删除建议删除指定范围内的行。改完文档后发出对应的 Accepted 信号，让调用方知道建议已被采纳。

		如果接受时目标行不在视口内，接受后会把视图滚动到实际改动的位置，否则用户看不到发生了什么。
	*/
	void TextEditPrivate::acceptSuggestion() {
		if (SgtKind == SuggestionKind::None) {
			return;
		}
		const SuggestionKind kind = SgtKind;
		const qint32 line = SgtLine;
		const qint32 removeSize = SgtRemoveSize;
		const bool pureAdd = SgtPureAdd;
		const QStringList current = (SgtIndex >= 0 and SgtIndex < SgtItems.size()) ? SgtItems.at(SgtIndex) : QStringList();
		bool ignored = false;
		const bool lineWasVisible = isSuggestionLineVisible(ignored);
		SgtKind = SuggestionKind::None;
		SgtLine = -1;
		SgtItems.clear();
		SgtIndex = 0;
		SgtRemoveSize = 0;
		SgtPureAdd = false;
		SgtTitleKey.clear();
		if (SuggestionWidget) {
			SuggestionWidget->hide();
		}
		ApplyingSuggestion = true;
		QTextDocument* document = Text->document();
		if (kind == SuggestionKind::Inline) {
			const QTextBlock block = document->findBlockByNumber(line);
			if (block.isValid()) {
				QTextCursor cursor(block);
				if (pureAdd) {
					// pureAdd：追加到行尾，保留原有内容。
					cursor.movePosition(QTextCursor::EndOfBlock);
					cursor.insertText(current.value(0));
				}
				else {
					cursor.movePosition(QTextCursor::StartOfBlock);
					cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
					cursor.insertText(current.value(0));
				}
				Text->setTextCursor(cursor);
			}
		}
		else if (kind == SuggestionKind::Block) {
			const QString text = current.join(QChar('\n'));
			QTextCursor cursor(document);
			if (line >= document->blockCount()) {
				// 追加到文档末尾：只有当前不在一个空行开头时才需要先补一个换行。
				cursor.movePosition(QTextCursor::End);
				if (not cursor.atBlockStart()) {
					cursor.insertText(QStringLiteral("\n") + text);
				}
				else {
					cursor.insertText(text);
				}
			}
			else {
				cursor = QTextCursor(document->findBlockByNumber(line));
				cursor.movePosition(QTextCursor::StartOfBlock);
				cursor.insertText(text + QChar('\n'));
			}
			Text->setTextCursor(cursor);
		}
		else {
			// 行删除：从目标行行首选到"删除范围之后那一行"的行首，连行带换行一起删掉。
			const qint32 blockCount = document->blockCount();
			const qint32 size = qMax(1, removeSize);
			const qint32 endLine = line + size;
			QTextCursor startCursor(document->findBlockByNumber(line));
			startCursor.movePosition(QTextCursor::StartOfBlock);
			int selectionStart = startCursor.position();
			int selectionEnd = 0;
			if (endLine < blockCount) {
				QTextCursor endCursor(document->findBlockByNumber(endLine));
				endCursor.movePosition(QTextCursor::StartOfBlock);
				selectionEnd = endCursor.position();
			}
			else {
				QTextCursor endCursor(document);
				endCursor.movePosition(QTextCursor::End);
				selectionEnd = endCursor.position();
				// 删到文档末尾时没有"下一行行首"，改为把目标行前面那个换行一起吃掉，否则会留下一个空行。
				selectionStart = qMax(0, selectionStart - 1);
			}
			QTextCursor cursor(document);
			cursor.setPosition(selectionStart);
			cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
			cursor.removeSelectedText();
			Text->setTextCursor(cursor);
		}
		ApplyingSuggestion = false;
		Text->viewport()->update();
		// 目标行原本在视口外时，接受后要转到真正改动的位置，否则用户面对的还是一屏无关内容。
		Text->ensureCursorVisible();
		if (not lineWasVisible) {
			// 尽量把它放到视口中部，而不是贴着上下边缘。
			Text->centerCursor();
		}
		if (kind == SuggestionKind::Inline) {
			emit q->modifySuggestionAccepted(line, current.value(0));
		}
		else if (kind == SuggestionKind::Block) {
			emit q->ghostTextAccepted(line, current);
		}
		else {
			emit q->blockRemoveAccepted(line, removeSize);
		}
	}

	/*!
		\a delta 在建议列表中移动的相对位置，-1 是上一个，1 是下一个。

		在待抉择状态下切换当前建议。只有一条建议时什么也不做。
	*/
	void TextEditPrivate::cycleSuggestion(qint32 delta) {
		if (SgtKind == SuggestionKind::None or SgtItems.size() <= 1 or delta == 0) {
			return;
		}
		const qint32 count = SgtItems.size();
		SgtIndex = ((SgtIndex + delta) % count + count) % count;
		updateSuggestionWidget();
		adjustSuggestionPosition();
		// pureAdd 的行内虚影画在行尾，切换建议后必须重绘文本区，否则看到的还是上一条。
		if (SgtKind == SuggestionKind::Inline and SgtPureAdd) {
			Text->viewport()->update();
		}
	}

	void TextEditPrivate::updateSuggestionWidget() {
		if (SuggestionWidget == nullptr) {
			return;
		}
		if (SgtKind == SuggestionKind::Remove) {
			// 删除建议没有候选内容，也不需要序号，只留标题和按钮。
			SuggestionWidget->setIndex(-1, 0);
			SuggestionWidget->setContent(QString());
		}
		else {
			SuggestionWidget->setIndex(SgtIndex + 1, SgtItems.size());
			SuggestionWidget->setContent(SgtItems.value(SgtIndex).join(QChar('\n')));
		}
		updateSuggestionTitle();
		SuggestionWidget->applySize();
	}

	/*!
		小标题随目标行是否在视口内变化：在视口内就写建议类型，否则改成"向上/向下滚动到行 xxx"并显示箭头。
		只有文案真的变了才重排浮窗，滚动时不会反复 setFixedSize。
	*/
	void TextEditPrivate::updateSuggestionTitle() {
		if (SuggestionWidget == nullptr or SgtKind == SuggestionKind::None) {
			return;
		}
		bool above = false;
		QString title;
		qint32 arrow = 0;
		if (isSuggestionLineVisible(above)) {
			switch (SgtKind) {
			case SuggestionKind::Inline:
				title = SgtPureAdd ? VITRL("YSS::editor.textEdit.suggestion.pureAdd")
					: VITRL("YSS::editor.textEdit.suggestion.inlineEdit");
				break;
			case SuggestionKind::Block:
				title = VITRL("YSS::editor.textEdit.suggestion.multiLineEdit");
				break;
			default:
				title = VITRL("YSS::editor.textEdit.suggestion.blockRemove");
				break;
			}
		}
		else {
			arrow = above ? -1 : 1;
			title = (above ? VITRL("YSS::editor.textEdit.suggestion.scrollUp")
				: VITRL("YSS::editor.textEdit.suggestion.scrollDown")).arg(SgtLine + 1);
		}
		const QString key = QStringLiteral("%1|%2").arg(arrow).arg(title);
		if (key == SgtTitleKey) {
			return;
		}
		SgtTitleKey = key;
		SuggestionWidget->setTitle(title);
		SuggestionWidget->setScrollArrow(arrow);
		SuggestionWidget->applySize();
	}

	/*!
		\return 当前建议的文字。行内建议取第一行，多行建议返回全部行；删除建议没有文字，返回空。
	*/
	QString TextEditPrivate::currentSuggestionText() const {
		if (SgtKind == SuggestionKind::Remove or SgtIndex < 0 or SgtIndex >= SgtItems.size()) {
			return QString();
		}
		return SgtItems.at(SgtIndex).join(QChar('\n'));
	}

	/*!
		\a above 目标行在视口上方时为 true，在视口下方时为 false（仅当返回 false 时有意义）。

		\return 目标行是否至少有一部分落在视口内。
	*/
	bool TextEditPrivate::isSuggestionLineVisible(bool& above) const {
		above = false;
		QTextDocument* document = Text->document();
		if (document == nullptr or document->blockCount() == 0 or SgtLine < 0) {
			return false;
		}
		const qint32 line = qBound(0, SgtLine, document->blockCount() - 1);
		const QTextBlock block = document->findBlockByNumber(line);
		if (not block.isValid()) {
			return false;
		}
		const QRect lineRect = Text->cursorRect(QTextCursor(block));
		if (not lineRect.isValid()) {
			return false;
		}
		const int viewportHeight = Text->viewport()->height();
		if (lineRect.bottom() < 0) {
			above = true;
			return false;
		}
		if (lineRect.top() >= viewportHeight) {
			above = false;
			return false;
		}
		return true;
	}

	/*!
		把建议浮窗贴到目标行右侧，并保证它不会跑出视口。
		行号越界（例如文档被截短）时直接退出待抉择状态。
	*/
	void TextEditPrivate::adjustSuggestionPosition() {
		if (SgtKind == SuggestionKind::None or SuggestionWidget == nullptr) {
			return;
		}
		QTextDocument* document = Text->document();
		if (document->blockCount() == 0 or SgtLine > document->blockCount() or SgtLine < 0) {
			dismissSuggestion(true);
			return;
		}
		const qint32 line = qBound(0, SgtLine, document->blockCount() - 1);
		const QTextBlock block = document->findBlockByNumber(line);
		if (not block.isValid()) {
			dismissSuggestion(true);
			return;
		}
		const QRect lineRect = Text->cursorRect(QTextCursor(block));
		const int margin = 8;
		// 滚动会改变目标行是否在视口内，小标题要跟着在"行内编辑建议"和"滚动到行 xxx"之间切换；
		// 这一步可能改变浮窗宽度，所以必须在算坐标之前做。
		updateSuggestionTitle();
		const int viewportWidth = Text->viewport()->width();
		const int viewportHeight = Text->viewport()->height();
		const int x = qMax(margin, viewportWidth - SuggestionWidget->width() - margin);
		// 右上角是测试按钮的地盘，浮窗只在这个位置让开它，其余情况仍与目标行对齐。
		const int avoidTop = TestButton ? TestButton->geometry().bottom() + 6 : margin;
		const int y = qBound(margin, qMax(lineRect.top(), avoidTop),
			qMax(margin, viewportHeight - SuggestionWidget->height() - margin));
		SuggestionWidget->move(x, y);
	}

	/*!
		测试按钮固定在编辑区右上角，跟随视口尺寸变化。
	*/
	void TextEditPrivate::positionTestButton() {
		if (TestButton == nullptr or Text == nullptr) {
			return;
		}
		TestButton->move(qMax(0, Text->viewport()->width() - TestButton->width() - 6), 4);
		TestButton->raise();
	}

	/*!
		临时测试入口：没有建议时在当前行弹出一条演示建议，有建议时收起。
		交替演示行内建议（多条，替换当前行）和多行建议（多条，在当前行之前插入）。
	*/
	void TextEditPrivate::runSuggestionDemo() {
		if (SgtKind != SuggestionKind::None) {
			dismissSuggestion(false);
			return;
		}
		QTextDocument* document = Text->document();
		const qint32 currentLine = Text->textCursor().blockNumber();
		switch (TestDemoIndex % 4) {
		case 0: {
			const qint32 line = qBound(0, currentLine, qMax(0, document->blockCount() - 1));
			q->showLineSuggestions(line, {
				QStringLiteral("音乐：immutable_ins/1"),
				QStringLiteral("背景：bg_amb2026/3"),
				QStringLiteral("遮罩：true") });
			break;
		}
		case 1: {
			const qint32 line = qBound(0, currentLine, qMax(0, document->blockCount() - 1));
			q->showLineSuggestions(line, {
				QStringLiteral(", 深度: PARADISE"),
				QStringLiteral("，透明度: 0.5") }, true);
			break;
		}
		case 2: {
			const qint32 line = qBound(0, currentLine, document->blockCount());
			q->showBlockSuggestions(line, {
				QStringList{
					QStringLiteral("？？？\t好安静的地方。"),
					QStringLiteral("？？？\t太安静了，以至于我无法判断这是好事还是坏事。") },
				QStringList{
					QStringLiteral("？？？\t看得到我吗？") } });
			break;
		}
		default: {
			const qint32 line = qBound(0, currentLine, qMax(0, document->blockCount() - 1));
			q->showBlockRemoveSuggestions(line, 2);
			break;
		}
		}
		++TestDemoIndex;
		// 浮窗本身是视口子控件，测试按钮后创建会被压在下面，这里重新抬起来。
		positionTestButton();
	}
}
