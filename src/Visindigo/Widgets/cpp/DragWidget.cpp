#include "Widgets/DragWidget.h"
#include <QtCore/qlist.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qboxlayout.h>
#include <QtGui/qevent.h>
#include "Widgets/private/DragWidget_p.h"
#include <QtWidgets/qapplication.h>
#include "General/Log.h"
namespace Visindigo::__Private__ {
	bool DragWidgetPrivate::eventFilter(QObject* watched, QEvent* event) {
		if (event->type() == QEvent::MouseButtonPress) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			DragStartPos = mouseEvent->globalPos();
			currentDraggingWidget = qApp->widgetAt(mouseEvent->globalPos());
		}
		else if (event->type() == QEvent::MouseMove) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint CurrentPos = mouseEvent->globalPos();
			if (not dragging) {
				if ((CurrentPos - DragStartPos).manhattanLength() > 10) {
					DragDeltaPos = CurrentPos - currentDraggingWidget->pos();
					dragging = true;
					onDragStart(mouseEvent);
					return true;
				}
			}
			else {
				onDragMove(mouseEvent);
			}
		}
		else if (event->type() == QEvent::MouseButtonRelease) {
			QWidget* widget = qobject_cast<QWidget*>(watched);
			if (dragging && widget) {
				dragging = false;
				onDragEnd(static_cast<QMouseEvent*>(event));
				currentDraggingWidget = nullptr;
				currentIndex = -1;
				return true;
			}
		}
		return QObject::eventFilter(watched, event);
	}

	void DragWidgetPrivate::onDragStart(QMouseEvent* event) {
		currentIndex = Widgets.indexOf(currentDraggingWidget);
		vgDebug << currentIndex;
		Layout->removeWidget(currentDraggingWidget);
		Widgets.removeAll(currentDraggingWidget);
		Widgets.insert(currentIndex, nullptr);
		if (not spaceWidget) {
			spaceWidget = new QWidget(q);
		}
		spaceWidget->show();
		if (LayoutOrientation == Qt::Vertical) {
			spaceWidget->setFixedHeight(currentDraggingWidget->height());
		}
		else {
			spaceWidget->setFixedWidth(currentDraggingWidget->width());
		}
		((QBoxLayout*)Layout)->insertWidget(currentIndex, spaceWidget);
		if (currentIndex != -1) {
			currentDraggingWidget->raise();
		}
		currentDraggingWidget->move(event->globalPos() - DragDeltaPos);

	}

	void DragWidgetPrivate::onDragMove(QMouseEvent* event) {
		if (currentDraggingWidget) {
			currentDraggingWidget->move(event->globalPos() - DragDeltaPos);
		}
	}

	void DragWidgetPrivate::onDragEnd(QMouseEvent* event) {
		if (currentDraggingWidget) { 
			currentDraggingWidget->move(event->globalPos() - DragDeltaPos);
		}
		currentDraggingWidget->hide();
		QWidget* targetWidget = qApp->widgetAt(event->globalPos());
		qint32 targetIndex;
		if (targetWidget != spaceWidget) {
			targetIndex = Widgets.indexOf(targetWidget)+1;
		}
		else {
			targetIndex = currentIndex;
		}
		vgDebug << targetIndex;
		((QBoxLayout*)Layout)->insertWidget(targetIndex, currentDraggingWidget);
		((QBoxLayout*)Layout)->removeWidget(spaceWidget);
		spaceWidget->hide();
		currentDraggingWidget->show();
		Widgets.insert(targetIndex, currentDraggingWidget);
		Widgets.removeAll(nullptr);
	}


}
namespace Visindigo::Widgets {
	DragWidget::DragWidget(QWidget* parent) :QFrame(parent), d(new Visindigo::__Private__::DragWidgetPrivate()) {
		d->q = this;
		if (d->LayoutOrientation == Qt::Vertical) {
			d->Layout = new QVBoxLayout(this);
		}
		else {
			d->Layout = new QHBoxLayout(this);
		}
	}

	DragWidget::~DragWidget() {
		delete d;
	}

	void DragWidget::setLayoutPolicy(Qt::Orientation orientation) {
		if (orientation == d->LayoutOrientation) return;
		d->LayoutOrientation = orientation;
		delete d->Layout;
		if (d->LayoutOrientation == Qt::Vertical) {
			d->Layout = new QVBoxLayout(this);
		}
		else {
			d->Layout = new QHBoxLayout(this);
		}
		for (QWidget* widget : d->Widgets) {
			d->Layout->addWidget(widget);
		}
	}

	void DragWidget::addWidget(QWidget* widget) {
		if (d->Widgets.contains(widget)) return;
		d->Widgets.append(widget);
		d->Layout->addWidget(widget);
		widget->installEventFilter(d);
	}

	void DragWidget::removeWidget(QWidget* widget) {
		if (!d->Widgets.contains(widget)) return;
		d->Widgets.removeAll(widget);
		d->Layout->removeWidget(widget);
		widget->removeEventFilter(d);
	}

	QBoxLayout* DragWidget::getBoxLayout() const {
		return (QBoxLayout*)d->Layout;
	}
}