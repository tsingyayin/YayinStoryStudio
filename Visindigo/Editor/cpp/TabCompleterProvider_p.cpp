#include "../private/TabCompleterProvider_p.h"
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextobject.h>
#include <QtCore/qregularexpression.h>
#include "../../Widgets/MultiButton.h"
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>
#include "../TabCompleterProvider.h"
#include "../../General/Log.h"
namespace Visindigo::__Private__ {
	TabCompleterWidget::TabCompleterWidget(QWidget* parent)
		: QFrame(parent) {
		this->setAttribute(Qt::WA_ShowWithoutActivating);
		this->setFocusPolicy(Qt::NoFocus);
		CentralWidget = new QWidget(this);
		ScrollArea = new QScrollArea(this);
		ScrollArea->setWidgetResizable(true);
		ScrollArea->setFocusPolicy(Qt::NoFocus);
		ScrollArea->setWidget(CentralWidget);
		ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		ScrollArea->verticalScrollBar()->setFixedWidth(16);
		Layout = new QVBoxLayout(CentralWidget);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(0);
		this->setFixedWidth(300);
		CentralWidget->setFixedWidth(300-16);
		ScrollArea->setFixedWidth(300);
		this->setStyleSheet("QFrame{border: 1px solid #666666; border-radius:5px}");
	}

	void TabCompleterWidget::setCompleterItems(const QList<Visindigo::Editor::TabCompleterItem>& items) {
		CompleterItems = items;
		for (QWidget* item : Items) {
			Layout->removeWidget(item);
			item->deleteLater();
		}
		Items.clear();
		CentralWidget->setFixedHeight(items.size() * 36);
		ScrollArea->setFixedHeight(qMin(items.size() * 36, 600));
		quint32 scrollWidth = ScrollArea->verticalScrollBar()->isVisible() ? 16 : 0;
		this->setFixedHeight(qMin(items.size() * 36, 600));
		yDebugF << items.size();
		for (Visindigo::Editor::TabCompleterItem item : items) {
			Visindigo::Widgets::MultiButton* button = new Visindigo::Widgets::MultiButton(CentralWidget);
			button->setTitle(item.getText());
			button->setPixmapPath(item.getIconPath());
			button->setPixmapFixedWidth(32);
			//button->setDescription(item.getDescription());
			yDebugF << item.getText() << item.getDescription();
			Layout->addWidget(button);
			button->show();
			button->setFixedHeight(36);
			button->setSpacing(2);
			button->setContentsMargins(2, 2, 2, 2);
			Items.append(button);
		}
	}
}