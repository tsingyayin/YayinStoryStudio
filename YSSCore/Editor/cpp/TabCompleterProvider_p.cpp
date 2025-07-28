#include "../private/TabCompleterProvider_p.h"
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextobject.h>
#include <QtCore/qregularexpression.h>
#include "../../Widgets/MultiButton.h"
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qboxlayout.h>
#include "../TabCompleterProvider.h"
#include "../../General/Log.h"
namespace YSSCore::__Private__ {
	TabCompleterWidget::TabCompleterWidget(QWidget* parent)
		: QFrame(parent) {
		this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
		//this->setAttribute(Qt::WA_TranslucentBackground);
		this->setAttribute(Qt::WA_ShowWithoutActivating);
		this->setFocusPolicy(Qt::NoFocus);
		CentralWidget = new QWidget(this);
		ScrollArea = new QScrollArea(this);
		ScrollArea->setWidgetResizable(true);
		ScrollArea->setFocusPolicy(Qt::NoFocus);
		ScrollArea->setWidget(CentralWidget);
		Layout = new QHBoxLayout(CentralWidget);
		Layout->setContentsMargins(0, 0, 0, 0);
		this->setFixedWidth(300);
	}

	void TabCompleterWidget::setCompleterItems(const QList<YSSCore::Editor::TabCompleterItem>& items) {
		CompleterItems = items;
		for (QWidget* item : Items) {
			Layout->removeWidget(item);
			item->deleteLater();
		}
		Items.clear();
		CentralWidget->setFixedHeight(items.size() * 30);
		for (YSSCore::Editor::TabCompleterItem item : items) {
			YSSCore::Widgets::MultiButton* button = new YSSCore::Widgets::MultiButton(CentralWidget);
			button->setTitle(item.getText());
			button->setDescription(item.getDescription());
			yDebugF << item.getText() << item.getDescription();
			Layout->addWidget(button);
			button->show();
			Items.append(button);
		}
	}
}