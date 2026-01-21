#include "Widgets/WidgetResizeTool.h"
#include <QtGui/qevent.h>
#include "General/Log.h"
namespace Visindigo::Widgets {
	class WidgetResizeToolPrivate :public QObject {
		friend class Visindigo::Widgets::WidgetResizeTool;
	protected:
		WidgetResizeTool::Borders EnabledBorders;
		int borderWidth;
		QPoint lastMousePos;
		bool resizing = false;
		bool onLeft = false;
		bool onRight = false;
		bool onTop = false;
		bool onBottom = false;
	};

	WidgetResizeTool::WidgetResizeTool(QWidget* parent, Borders borders, int borderWidth) :QObject((QObject*)parent) {
		d = new WidgetResizeToolPrivate();
		d->EnabledBorders = borders;
		d->borderWidth = borderWidth;
		parent->installEventFilter(this);
		parent->setMouseTracking(true);
	}

	WidgetResizeTool::~WidgetResizeTool() {
		delete d;
	}

	void WidgetResizeTool::setEnableBorder(Borders borders) {
		d->EnabledBorders = borders;
	}

	bool WidgetResizeTool::eventFilter(QObject* target, QEvent* event) {
		if (target!=this->parent()) {
			return QObject::eventFilter(target, event);
		}
		QWidget* parentWidget = qobject_cast<QWidget*>(target);
		if (parentWidget != nullptr && event->type() == QEvent::MouseMove) {
			if (!parentWidget->isEnabled()) {
				return QObject::eventFilter(target, event);
			}
			if (parentWidget->isMaximized() || parentWidget->isFullScreen()) {
				parentWidget->unsetCursor();
				return QObject::eventFilter(target, event);
			}
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPointF pos = mouseEvent->position();
			QRect rect = parentWidget->rect();
			Qt::CursorShape cursorShape = Qt::ArrowCursor;
			if (!d->resizing) {
				d->onLeft = pos.x() <= d->borderWidth;
				d->onRight = pos.x() >= rect.width() - d->borderWidth;
				d->onTop = pos.y() <= d->borderWidth;
				d->onBottom = pos.y() >= rect.height() - d->borderWidth;
			}
			if (!d->onLeft && !d->onRight && !d->onTop && !d->onBottom) {
				parentWidget->unsetCursor();
				//vgDebug << "Unset Cursor";
				return false;
			}
			if (d->onLeft && d->onTop && d->EnabledBorders.testAnyFlag(LeftTop)) {
				cursorShape = Qt::SizeFDiagCursor;
			}
			else if (d->onRight && d->onTop && d->EnabledBorders.testAnyFlag(RightTop)) {
				cursorShape = Qt::SizeBDiagCursor;
			}
			else if (d->onLeft && d->onBottom && d->EnabledBorders.testAnyFlag(LeftBottom)) {
				cursorShape = Qt::SizeBDiagCursor;
			}
			else if (d->onRight && d->onBottom && d->EnabledBorders.testAnyFlag(RightBottom)) {
				cursorShape = Qt::SizeFDiagCursor;
			}
			else if (d->onLeft && d->EnabledBorders.testAnyFlag(Left)) {
				cursorShape = Qt::SizeHorCursor;
			}
			else if (d->onRight && d->EnabledBorders.testAnyFlag(Right)) {
				cursorShape = Qt::SizeHorCursor;
			}
			else if (d->onTop && d->EnabledBorders.testAnyFlag(Top)) {
				cursorShape = Qt::SizeVerCursor;
			}
			else if (d->onBottom && d->EnabledBorders.testAnyFlag(Bottom)) {
				cursorShape = Qt::SizeVerCursor;
			}

			if (cursorShape == Qt::ArrowCursor) {
				parentWidget->unsetCursor();
				d->resizing = false;
				return false;
			}else{
				if (mouseEvent->buttons() == Qt::LeftButton) {
					if (!d->resizing) {
						d->resizing = true;
						d->lastMousePos = mouseEvent->globalPos();
						return false; // cannot resize on the first move event
					}
					if (cursorShape != Qt::ArrowCursor) {
						QPoint delta = mouseEvent->globalPos() - d->lastMousePos;
						//vgDebug << "Delta:" << delta.x() << delta.y();
						QRect newGeometry = parentWidget->geometry();
						if (cursorShape == Qt::SizeFDiagCursor) {
							if (d->onLeft) {
								newGeometry.setLeft(newGeometry.left() + delta.x());
							}
							else {
								newGeometry.setRight(newGeometry.right() + delta.x());
							}
							if (d->onTop) {
								newGeometry.setTop(newGeometry.top() + delta.y());
							}
							else {
								newGeometry.setBottom(newGeometry.bottom() + delta.y());
							}
						}
						else if (cursorShape == Qt::SizeBDiagCursor) {
							if (d->onLeft) {
								newGeometry.setLeft(newGeometry.left() + delta.x());
							}
							else {
								newGeometry.setRight(newGeometry.right() + delta.x());
							}
							if (d->onBottom) {
								newGeometry.setBottom(newGeometry.bottom() + delta.y());
							}
							else {
								newGeometry.setTop(newGeometry.top() + delta.y());
							}
						}
						else if (cursorShape == Qt::SizeHorCursor) {
							if (d->onLeft) {
								newGeometry.setLeft(newGeometry.left() + delta.x());
							}
							else {
								newGeometry.setRight(newGeometry.right() + delta.x());
							}
						}
						else if (cursorShape == Qt::SizeVerCursor) {
							if (d->onTop) {
								newGeometry.setTop(newGeometry.top() + delta.y());
							}
							else {
								newGeometry.setBottom(newGeometry.bottom() + delta.y());
							}
						}
						d->lastMousePos = mouseEvent->globalPos();
						parentWidget->setGeometry(newGeometry);
					}
				}
				else {
					d->resizing = false;
				}
			}
			parentWidget->setCursor(cursorShape);
			return false;	
		}
		else {

			return QObject::eventFilter(target, event);
		}
		return QObject::eventFilter(target, event);
	}
}