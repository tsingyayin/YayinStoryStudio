#include "../WidgetDisplayStack.h"
#include <QtCore/qstack.h>
#include <QtWidgets/qwidget.h>

namespace YSSCore::Widgets {
	class WidgetDisplayStackPrivate {
		friend class WidgetDisplayStack;
		QStack<QWidget*> StackWidgets;
		QList<QWidget*> OwnerPtrs;
	};

	WidgetDisplayStack::WidgetDisplayStack() {
		d = new WidgetDisplayStackPrivate;
	}

	bool  WidgetDisplayStack::pushWidget(QWidget* widget, bool deleteOnClose, bool ownerPtr) {
		if (d->StackWidgets.contains(widget)) {
			return false;
		}
		d->StackWidgets.top()->hide();
		d->StackWidgets.push(widget);
		if (deleteOnClose) {
			widget->setAttribute(Qt::WA_DeleteOnClose);
		}
		if (ownerPtr) {
			d->OwnerPtrs.append(widget);
		}
		widget->show();
		return true;
	}

	QWidget* WidgetDisplayStack::popWidget() {
		d->StackWidgets.top()->close();
		QWidget* poped = d->StackWidgets.pop();
		if (d->OwnerPtrs.contains(poped)) {
			delete poped;
			poped = nullptr;
		}
		return d->StackWidgets.top();
	}
}