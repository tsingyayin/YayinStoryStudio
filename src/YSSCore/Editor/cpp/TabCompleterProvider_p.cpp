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
#include <General/TranslationHost.h>
namespace YSSCore::__Private__ {
	TabCompleterWidget::TabCompleterWidget(YSSCore::Editor::TextEdit* textEdit)
		: Visindigo::Widgets::BorderFrame(textEdit) {
		Target = textEdit;
		this->setAttribute(Qt::WA_ShowWithoutActivating);
		this->setFocusPolicy(Qt::NoFocus);
		this->setFixedWidth(300);
		auto font = this->font();
		font.setPointSizeF(font.pointSizeF() * 0.9);
		this->setFont(font);
		this->DescriptionLabel = new Visindigo::Widgets::BorderLabel(this);
		this->DescriptionLabel->setMinimumHeight(32);
		this->ButtonGroup = new Visindigo::Widgets::MultiButtonGroup(this);
		connect(ButtonGroup, &Visindigo::Widgets::MultiButtonGroup::doubleClicked, this, &TabCompleterWidget::doComplete);
		connect(ButtonGroup, &Visindigo::Widgets::MultiButtonGroup::selectIndexChanged, this, [this](qint32 index) {
			if (index == -1) return;
			qint32 globalIndex = ButtonCycleIndexes[index] * ButtonCycleIndexes.size() + index;
			this->currentSelectedIndex = globalIndex;
			this->DescriptionLabel->setText(Items[globalIndex].getDescription());
			});

		Layout = new QVBoxLayout(this);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(0);
		ScrollContainer = new QWidget(this);
		ScrollContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		Layout->addWidget(ScrollContainer);
		ScrollBar = new QScrollBar(Qt::Vertical, ScrollContainer);
		ScrollBar->setVisible(true);
		ScrollBar->setSingleStep(120);
		connect(ScrollBar, &QScrollBar::valueChanged, this, &TabCompleterWidget::onScrollValueChanged);
		maxAllowedHeight = (buttonCacheSize - 4) * buttonHeight;
		this->setFixedHeight(maxAllowedHeight);
		ScrollBar->setGeometry(300 - 16, 0, 16, ScrollContainer->height());
		ScrollBar->setRange(0, 0);
		for (int i = 0; i < buttonCacheSize; i++) {
			Visindigo::Widgets::MultiButton* button = new Visindigo::Widgets::MultiButton(ScrollContainer);
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
		TypeToolBar = new QToolBar(this);
		Layout->addWidget(TypeToolBar);
		Layout->addWidget(DescriptionLabel);
		ValueFilter = new QAction(VITR("YSS::editor.textEdit.tabCompleter.value"), this);
		ValueFilter->setIcon(QIcon(Editor::TabCompleterItem::getTypeIconPath(YSSCore::Editor::TabCompleterItem::Value)));
		ConstFilter = new QAction(VITR("YSS::editor.textEdit.tabCompleter.const"), this);
		ConstFilter->setIcon(QIcon(Editor::TabCompleterItem::getTypeIconPath(YSSCore::Editor::TabCompleterItem::Const)));
		EnumFilter = new QAction(VITR("YSS::editor.textEdit.tabCompleter.enum"), this);
		EnumFilter->setIcon(QIcon(Editor::TabCompleterItem::getTypeIconPath(YSSCore::Editor::TabCompleterItem::Enum)));
		FunctionFilter = new QAction(VITR("YSS::editor.textEdit.tabCompleter.function"), this);
		FunctionFilter->setIcon(QIcon(Editor::TabCompleterItem::getTypeIconPath(YSSCore::Editor::TabCompleterItem::Function)));
		ObjectFilter = new QAction(VITR("YSS::editor.textEdit.tabCompleter.object"), this);
		ObjectFilter->setIcon(QIcon(Editor::TabCompleterItem::getTypeIconPath(YSSCore::Editor::TabCompleterItem::Object)));
		OperatorFilter = new QAction(VITR("YSS::editor.textEdit.tabCompleter.operator"), this);
		OperatorFilter->setIcon(QIcon(Editor::TabCompleterItem::getTypeIconPath(YSSCore::Editor::TabCompleterItem::Operator)));
		ValueFilter->setCheckable(true);
		ConstFilter->setCheckable(true);
		EnumFilter->setCheckable(true);
		FunctionFilter->setCheckable(true);
		ObjectFilter->setCheckable(true);
		OperatorFilter->setCheckable(true);
		TypeToolBar->addActions({ ValueFilter, ConstFilter, EnumFilter, FunctionFilter, ObjectFilter, OperatorFilter });
		connect(ValueFilter, &QAction::toggled, this, &TabCompleterWidget::onFilterButtonToggled);
		connect(ConstFilter, &QAction::toggled, this, &TabCompleterWidget::onFilterButtonToggled);
		connect(EnumFilter, &QAction::toggled, this, &TabCompleterWidget::onFilterButtonToggled);
		connect(FunctionFilter, &QAction::toggled, this, &TabCompleterWidget::onFilterButtonToggled);
		connect(ObjectFilter, &QAction::toggled, this, &TabCompleterWidget::onFilterButtonToggled);
		connect(OperatorFilter, &QAction::toggled, this, &TabCompleterWidget::onFilterButtonToggled);
		setColorfulEnable(true);
		onThemeChanged();
	}

	void TabCompleterWidget::setCompleterItems(const QList<YSSCore::Editor::TabCompleterItem>& items) {
		RawItems = items;
		reApplyFilter();
	}

	void TabCompleterWidget::reApplyFilter() {
		Items.clear();
		auto filter = Target->getCompleterTypeFilter();
		for (const auto& item : RawItems) {
			if (filter.testAnyFlag(item.getType())) {
				Items.append(item);
			}
		}
		for (int i = 0; i < ButtonCycleIndexes.size(); i++) {
			ButtonCycleIndexes[i] = -1;
		}
		asyncFilterButton();
		this->ScrollBar->setRange(0, Items.size() * buttonHeight);
		this->ScrollBar->setValue(0);
		this->currentSelectedIndex = 0;
		this->ButtonGroup->selectButton(0);
		this->onScrollValueChanged(0);
	}

	void TabCompleterWidget::asyncFilterButton() {
		auto filter = Target->getCompleterTypeFilter();
		ValueFilter->setChecked(filter.testFlag(YSSCore::Editor::TabCompleterItem::Value));
		ConstFilter->setChecked(filter.testFlag(YSSCore::Editor::TabCompleterItem::Const));
		EnumFilter->setChecked(filter.testFlag(YSSCore::Editor::TabCompleterItem::Enum));
		FunctionFilter->setChecked(filter.testFlag(YSSCore::Editor::TabCompleterItem::Function));
		ObjectFilter->setChecked(filter.testFlag(YSSCore::Editor::TabCompleterItem::Object));
		OperatorFilter->setChecked(filter.testFlag(YSSCore::Editor::TabCompleterItem::Operator));
	}

	void TabCompleterWidget::onFilterButtonToggled(bool checked) {
		auto filter = Target->getCompleterTypeFilter();
		filter.setFlag(YSSCore::Editor::TabCompleterItem::Value, ValueFilter->isChecked());
		filter.setFlag(YSSCore::Editor::TabCompleterItem::Const, ConstFilter->isChecked());
		filter.setFlag(YSSCore::Editor::TabCompleterItem::Enum, EnumFilter->isChecked());
		filter.setFlag(YSSCore::Editor::TabCompleterItem::Function, FunctionFilter->isChecked());
		filter.setFlag(YSSCore::Editor::TabCompleterItem::Object, ObjectFilter->isChecked());
		filter.setFlag(YSSCore::Editor::TabCompleterItem::Operator, OperatorFilter->isChecked());
		Target->setCompleterTypeFilter(filter);
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
		QTextCursor cursor = Target->getTextCursor();
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
		ScrollBar->setGeometry(300 - 16, 0, 16, ScrollContainer->height());
	}

	void TabCompleterWidget::resizeEvent(QResizeEvent* event) {
		ScrollBar->setGeometry(300 - 16, 0, 16, ScrollContainer->height());
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