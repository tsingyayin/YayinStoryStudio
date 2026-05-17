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
		this->ButtonGroup = new Visindigo::Widgets::MultiButtonGroup(this);
		connect(ButtonGroup, &Visindigo::Widgets::MultiButtonGroup::doubleClicked, this, &TabCompleterWidget::doComplete);
		connect(ButtonGroup, &Visindigo::Widgets::MultiButtonGroup::selectIndexChanged, this, [this](qint32 index) {
			if (index == -1) return;
			qint32 globalIndex = ButtonCycleIndexes[index] * ButtonCycleIndexes.size() + index;
			this->currentSelectedIndex = globalIndex;
			});
		ScrollBar = new QScrollBar(Qt::Vertical, this);
		ScrollBar->setVisible(true);
		ScrollBar->setSingleStep(120);
		connect(ScrollBar, &QScrollBar::valueChanged, this, &TabCompleterWidget::onScrollValueChanged);
		maxAllowedHeight = (buttonCacheSize - 4) * buttonHeight;
		this->setFixedHeight(maxAllowedHeight);
		ScrollBar->setGeometry(300 - 16, 0, 16, this->height());
		ScrollBar->setRange(0, 0);
		for(int i = 0; i < buttonCacheSize; i++) {
			Visindigo::Widgets::MultiButton* button = new Visindigo::Widgets::MultiButton(this);
			button->setTitle("");
			button->setPixmapPath("");
			button->setPixmapFixedWidth(buttonHeight - 4);
			button->setFixedHeight(buttonHeight);
			button->setFixedWidth(300 - 16);
			button->setSpacing(2);
			button->setContentsMargins(2, 2, 2, 2);
			Buttons.append(button);
			ButtonGroup->addButton(button);
			ButtonCycleIndexes.append(-1);
		}
		setColorfulEnable(true);
		onThemeChanged();
	}

	void TabCompleterWidget::setCompleterItems(const QList<YSSCore::Editor::TabCompleterItem>& items) {
		Items = items;
		for (int i = 0; i < ButtonCycleIndexes.size(); i++) {
			ButtonCycleIndexes[i] = -1;
		}
		this->ScrollBar->setRange(0, items.size() * buttonHeight);
		this->ScrollBar->setValue(0);
		this->currentSelectedIndex = 0;
		this->ButtonGroup->selectButton(0);
		this->onScrollValueChanged(0);
	}

	void TabCompleterWidget::selectPrevious() {
		qint32 oldY = ScrollBar->value();
		ScrollBar->setValue(oldY - buttonHeight);
		if (currentSelectedIndex != 0) { 
			ButtonGroup->selectPrevious();
			currentSelectedIndex--;
		}
	}

	void TabCompleterWidget::selectNext() {
		qint32 oldY = ScrollBar->value();
		ScrollBar->setValue(oldY + buttonHeight);
		if (currentSelectedIndex != Items.size() - 1) {
			ButtonGroup->selectNext();
			currentSelectedIndex++;
		}
	}

	void TabCompleterWidget::doComplete(Visindigo::Widgets::MultiButton* pressed) {
		if (pressed == nullptr) {
			pressed = ButtonGroup->getSelectedButton();
		}
		if (!pressed) return;
		qint32 localindex = Buttons.indexOf(pressed);
		qint32 globalIndex = ButtonCycleIndexes[localindex] * ButtonCycleIndexes.size() + localindex;
		auto item = Items[globalIndex];
		if (item.getContent().isEmpty()) return;
		QTextCursor cursor = Target->textCursor();
		yDebugF << "Complete Content:" << item.isAlignment();
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
		yDebugF << "Do Complete";
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
				if (ButtonGroup->getSelectedButton() == Buttons[i]) {
					ButtonGroup->selectButton(-1);
				}
				ButtonCycleIndexes[i] = -1;
				continue;
			}
			if (ButtonCycleIndexes[i] != firstIndex + indexDelta) {
				auto item = Items[correctIndex];
				Buttons[i]->setTitle(item.getText());
				Buttons[i]->setPixmapPath(item.getIconPath());
				Buttons[i]->show();
				if (ButtonGroup->getSelectedButton() == Buttons[i]) {
					ButtonGroup->selectButton(-1);
				}
				qint32 globalIndex = (firstIndex + indexDelta) * ButtonCycleIndexes.size() + i;
				if (globalIndex == this->currentSelectedIndex) {
					ButtonGroup->selectButton(Buttons[i]);
				}
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

	void TabCompleterWidget::onThemeChanged(){
		for (auto button : Buttons) {
			button->setNormalStyleSheet(VISTMGT("YSS::General.MultiButton.Pressed", button));
			button->setHoverStyleSheet(VISTMGT("YSS::General.MultiButton.Normal", button));
			button->setPressedStyleSheet(VISTMGT("YSS::General.MultiButton.Hover", button));
		}
		this->update();
	}
}