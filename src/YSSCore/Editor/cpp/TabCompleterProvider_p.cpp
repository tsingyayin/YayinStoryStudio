#include <QtGui/qtextcursor.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qtextedit.h>
#include <General/Log.h>
#include <Widgets/MultiButton.h>
#include <Widgets/MultiButtonGroup.h>
#include <Widgets/ThemeManager.h>
#include "../private/TabCompleterProvider_p.h"
#include "../TabCompleterProvider.h"
#include "General/YSSLogger.h"
namespace YSSCore::__Private__ {
	TabCompleterWidget::TabCompleterWidget(QTextEdit* textEdit)
		: QFrame(textEdit) {
		this->TextEdit = textEdit;
		this->setAttribute(Qt::WA_ShowWithoutActivating);
		this->setFocusPolicy(Qt::NoFocus);
		this->ButtonGroup = new Visindigo::Widgets::MultiButtonGroup(this);
		connect(ButtonGroup, &Visindigo::Widgets::MultiButtonGroup::doubleClicked, this, &TabCompleterWidget::doComplete);
		CentralWidget = new QWidget(this);
		ScrollArea = new QScrollArea(this);
		ScrollArea->setWidgetResizable(true);
		ScrollArea->setFocusPolicy(Qt::NoFocus);
		ScrollArea->setWidget(CentralWidget);
		ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		ScrollArea->verticalScrollBar()->setFixedWidth(16);
		ScrollArea->verticalScrollBar()->setMinimum(0);
		Layout = new QVBoxLayout(CentralWidget);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(0);
		this->setFixedWidth(300);
		CentralWidget->setFixedWidth(300 - 16);
		ScrollArea->setFixedWidth(300);
		this->setStyleSheet(VISTMGT("YSS::Default"));
		this->ScrollArea->verticalScrollBar()->setStyleSheet(VISTMGT("YSS::NormalScrollBar", this));
	}

	void TabCompleterWidget::setCompleterItems(const QList<YSSCore::Editor::TabCompleterItem>& items) {
		CompleterItems = items;
		for (QWidget* item : Items) {
			Layout->removeWidget(item);
			item->deleteLater();
		}
		Items.clear();
		ButtonGroup->removeAll();
		ItemMap.clear();
		CentralWidget->setFixedHeight(items.size() * 36);
		ScrollArea->setFixedHeight(qMin(items.size() * 36, 600));
		ScrollArea->verticalScrollBar()->setValue(0);
		ScrollArea->verticalScrollBar()->setMaximum(items.size() * 36);
		quint32 scrollWidth = ScrollArea->verticalScrollBar()->isVisible() ? 16 : 0;
		this->setFixedHeight(qMin(items.size() * 36, 600));
		//yDebugF << items.size();
		for (YSSCore::Editor::TabCompleterItem item : items) {
			Visindigo::Widgets::MultiButton* button = new Visindigo::Widgets::MultiButton(CentralWidget);
			button->setTitle(item.getText());
			button->setPixmapPath(item.getIconPath());
			button->setPixmapFixedWidth(32);
			//yDebugF << item.getText() << item.getDescription();
			Layout->addWidget(button);
			button->show();
			button->setFixedHeight(36);
			button->setSpacing(2);
			button->setContentsMargins(2, 2, 2, 2);
			button->setNormalStyleSheet(VISTMGT("YSS::TextEdit.TabCompleter.Normal", button));
			button->setHoverStyleSheet(VISTMGT("YSS::TextEdit.TabCompleter.Hover", button));
			button->setPressedStyleSheet(VISTMGT("YSS::TextEdit.TabCompleter.Pressed", button));
			Items.append(button);
			ItemMap.insert(button, item);
			ButtonGroup->addButton(button);
		}
		if (Items.size() > 0) {
			ButtonGroup->selectButton(0);
		}
	}

	void TabCompleterWidget::selectPrevious() {
		qint32 index = ButtonGroup->selectPrevious();
		ScrollArea->verticalScrollBar()->setValue(index * 36);
	}

	void TabCompleterWidget::selectNext() {
		qint32 index = ButtonGroup->selectNext();
		ScrollArea->verticalScrollBar()->setValue(index * 36);
	}

	void TabCompleterWidget::doComplete(Visindigo::Widgets::MultiButton* pressed) {
		if (pressed == nullptr) {
			pressed = ButtonGroup->getSelectedButton();
		}
		if (!pressed) return;
		YSSCore::Editor::TabCompleterItem item = ItemMap.value(pressed);
		if (item.getContent().isEmpty()) return;
		QTextCursor cursor = TextEdit->textCursor();
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
}