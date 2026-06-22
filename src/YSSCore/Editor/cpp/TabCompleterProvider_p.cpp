#include <QtGui/qtextcursor.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qtextedit.h>
#include <QtGui/qevent.h>
#include <General/Log.h>
#include <Widgets/MultiButton.h>
#include <Widgets/MultiButtonGroup.h>
#include <Widgets/ThemeManager.h>
#include "../private/TabCompleterProvider_p.h"
#include "../TabCompleterProvider.h"
#include "General/YSSLogger.h"
namespace YSSCore::__Private__ {
	TabCompleterWidget::TabCompleterWidget(QTextEdit* textEdit)
		: Visindigo::Widgets::BorderFrame(textEdit) {
		Target = textEdit;
		this->setAttribute(Qt::WA_ShowWithoutActivating);
		this->setFocusPolicy(Qt::NoFocus);
		this->setFixedWidth(300);
		auto font = this->font();
		font.setPointSizeF(font.pointSizeF() * 0.9);
		this->setFont(font);
		ScrollBar = new QScrollBar(Qt::Vertical, this);
		ScrollBar->setVisible(true);
		ScrollBar->setSingleStep(120);
		connect(ScrollBar, &QScrollBar::valueChanged, this, &TabCompleterWidget::onScrollValueChanged);
		maxAllowedHeight = (buttonCacheSize - 4) * buttonHeight;
		this->setFixedHeight(maxAllowedHeight);
		ScrollBar->setGeometry(300 - 16, 0, 16, this->height());
		ScrollBar->setRange(0, 0);
		for (int i = 0; i < buttonCacheSize; i++) {
			Visindigo::Widgets::MultiButton* button = new Visindigo::Widgets::MultiButton(this);
			button->setCheckedEnable(true);
			button->setUseItemStyle(true);
			button->setTitle("");
			button->setPixmapPath("");
			button->setPixmapFixedWidth(buttonHeight - 4);
			button->setFixedHeight(buttonHeight);
			button->setFixedWidth(300 - 16);
			button->setSpacing(2);
			button->setContentsMargins(2, 2, 2, 2);
			Buttons.append(button);
			ButtonCycleIndexes.append(-1);
			connect(button, &Visindigo::Widgets::MultiButton::doubleClicked, this, [this, button]() {
				this->doComplete(button);
				});
			connect(button, &Visindigo::Widgets::MultiButton::clicked, this, [this, button, i]() {
				this->currentSelectedIndex = ButtonCycleIndexes[i] * ButtonCycleIndexes.size() + i;
				});
			connect(button, &Visindigo::Widgets::MultiButton::checked, this, [this, button](bool checked) {
				if (checked) {
					for (auto other : Buttons) {
						if (other != button) {
							other->setChecked(false);
						}
					}
				}
				});
		}
		setColorfulEnable(true);
		onThemeChanged();
	}

	void TabCompleterWidget::setCompleterItems(const QList<YSSCore::Editor::TabCompleterItem>& items) {
		Items = items;
		for (int i = 0; i < ButtonCycleIndexes.size(); i++) {
			ButtonCycleIndexes[i] = -1;
		}
		this->ScrollBar->setRange(0, (items.size()-1) * buttonHeight);
		this->ScrollBar->setValue(0);
		this->currentSelectedIndex = 0;
		this->Buttons[0]->setChecked(true);
		this->onScrollValueChanged(0);
	}

	void TabCompleterWidget::selectPrevious() {
		if (Items.size() == 0) return;
		if (currentSelectedIndex != 0) {
			currentSelectedIndex--;
		}
		else {
			currentSelectedIndex = Items.size() - 1;
		}
		//vgDebugF << "Select Previous, Current Selected Index:" << currentSelectedIndex;
		qint32 tY = currentSelectedIndex * buttonHeight;
		if (tY < 0) { tY = ScrollBar->maximum() - buttonHeight; }
		ScrollBar->setValue(tY);
	}

	void TabCompleterWidget::selectNext() {
		if (Items.size() == 0) return;
		if (currentSelectedIndex != Items.size() - 1) {
			currentSelectedIndex++;
		}
		else {
			currentSelectedIndex = 0;
		}
		//vgDebugF << "Select Next, Current Selected Index:" << currentSelectedIndex;
		qint32 tY = currentSelectedIndex * buttonHeight;
		if (tY > ScrollBar->maximum()) { tY = 0; }
		ScrollBar->setValue(tY);
	}

	void TabCompleterWidget::doComplete(Visindigo::Widgets::MultiButton* pressed) {
		if (not pressed) {
			pressed = Buttons[currentSelectedIndex % ButtonCycleIndexes.size()];
		}
		qint32 localindex = Buttons.indexOf(pressed);
		qint32 globalIndex = ButtonCycleIndexes[localindex] * ButtonCycleIndexes.size() + localindex;
		if (globalIndex < 0 || globalIndex >= Items.size()) return;
		auto item = Items[globalIndex];
		if (item.getContent().isEmpty()) return;
		QTextCursor cursor = Target->textCursor();
		yDebugF << "Complete Content:" << item.getContent();
		if (item.isAlignment()) {
			QString selected;
			while (cursor.positionInBlock() != 0) {
				cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
				selected = cursor.selectedText();
				if (item.getContent().startsWith(selected)) {
					cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, selected.length());
					cursor.insertText(item.getContent().right(item.getContent().length() - selected.length()));
					yDebugF << "Found" << selected << " exists, insert the rest";
					return;
				}
			}
			cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, selected.length());
			cursor.insertText(item.getContent());
		}
		else {
			cursor.insertText(item.getContent());
		}
		//yDebugF << "Do Complete";
	}

	void TabCompleterWidget::scrollBy(qint32 y) {
		ScrollBar->setValue(ScrollBar->value() + y);
	}

	void TabCompleterWidget::onScrollValueChanged(qint32 value) {
		
		qint32 firstIndex = value / (buttonHeight * ButtonCycleIndexes.size());
		qint32 deltaY = value % (buttonHeight * ButtonCycleIndexes.size());
		//vgDebug << "Scroll Value Changed:" << value << "First Index:" << firstIndex << "DeltaY:" << deltaY;
		for (int i = 0; i < Buttons.size(); i++) {
			Buttons[i]->move(0, i * buttonHeight - deltaY);
			qint32 indexDelta = 0;
			if (Buttons[i]->y() < -buttonHeight) {
				indexDelta++;
				Buttons[i]->move(0, i * buttonHeight - deltaY + ButtonCycleIndexes.size() * buttonHeight);
			}
			qint32 correctIndex = (firstIndex + indexDelta) * ButtonCycleIndexes.size() + i;
			if (correctIndex >= Items.size()) {
				Buttons[i]->setTitle("");
				Buttons[i]->setPixmapPath("");
				Buttons[i]->hide();
				ButtonCycleIndexes[i] = -1;
				Buttons[i]->setChecked(false);
				continue;
			}
			if (ButtonCycleIndexes[i] != firstIndex + indexDelta) {
				auto item = Items[correctIndex];
				Buttons[i]->setTitle(item.getText());
				Buttons[i]->setPixmapPath(item.getIconPath());
				Buttons[i]->show();
			}
			qint32 globalIndex = (firstIndex + indexDelta) * ButtonCycleIndexes.size() + i;
			if (globalIndex == this->currentSelectedIndex) {
				Buttons[i]->setChecked(true);
				//vgDebugF << "Set Button" << i << "Checked, Global Index:" << globalIndex;
			}else{
				Buttons[i]->setChecked(false);
			}
			ButtonCycleIndexes[i] = firstIndex + indexDelta;
		}
	}

	void TabCompleterWidget::adjustHeight(qint32 height) {
		this->setFixedHeight(height);
		ScrollBar->setGeometry(300 - 16, 0, 16, height);
	}

	qint32 TabCompleterWidget::getMaxAllowedHeight() const {
		return maxAllowedHeight;
	}

	void TabCompleterWidget::wheelEvent(QWheelEvent* event) {
		this->scrollBy(-event->angleDelta().y());
	}

	void TabCompleterWidget::onThemeChanged() {
		for (auto button : Buttons) {
			//button->setNormalStyleSheet(VISTMGT("YSS::General.MultiButton.Pressed", button));
			//button->setHoverStyleSheet(VISTMGT("YSS::General.MultiButton.Normal", button));
			//button->setPressedStyleSheet(VISTMGT("YSS::General.MultiButton.Hover", button));
		}
		this->update();
	}
}