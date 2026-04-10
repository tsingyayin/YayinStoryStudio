#include "Widgets/WidgetResizeTool.h"
#include <QtGui/qevent.h>
#include <QtCore/qtimer.h>
#include "General/Log.h"
namespace Visindigo::Widgets {
	class WidgetResizeToolPrivate :public QObject {
		friend class Visindigo::Widgets::WidgetResizeTool;
	protected:
		QWidget* topParent = nullptr;
		QTimer* mouseChecker = nullptr;
		WidgetResizeTool::Borders EnabledBorders;
		int borderWidth;
		QPoint lastMousePos;
		bool resizing = false;
		bool onLeft = false;
		bool onRight = false;
		bool onTop = false;
		bool onBottom = false;

		bool mouseInBorderArea(const QPointF& pos, const QRect& rect) {
			bool onLeft = pos.x() <= borderWidth;
			bool onRight = pos.x() >= rect.width() - borderWidth;
			bool onTop = pos.y() <= borderWidth;
			bool onBottom = pos.y() >= rect.height() - borderWidth;
			return onLeft || onRight || onTop || onBottom;
		}
	};

	/*!
		\class Visindigo::Widgets::WidgetResizeTool
		\brief WidgetResizeTool提供了一个为任意QWidget通过拖动边框调整大小的工具。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		WidgetResizeTool可以安装在任何QWidget上，使其能够通过拖动边框来调整大小。
		用户可以指定哪些边框可以调整大小，以及边框的宽度。该工具会自动检测鼠标位置，
		并在鼠标悬停在可调整大小的边框区域时显示相应的调整大小光标。当用户按下鼠标左键并拖动时，
		工具会根据鼠标移动的距离调整窗口的大小。

		\warning 由于Qt的事件传播机制，这个类有已知缺陷，不能正确处理鼠标图标
		以及部分情况下的调整行为。建议不要用这个类，它可能在未来的版本中被移除。
	*/

	/*!
		\enum Visindigo::Widgets::WidgetResizeTool::Border
		\value Left 可调整左边框
		\value Top 可调整上边框
		\value Right 可调整右边框
		\value Bottom 可调整下边框
		\value LeftTop 可调整左上角
		\value RightTop 可调整右上角
		\value LeftBottom 可调整左下角
		\value RightBottom 可调整右下角
		\value AllBorder 可调整所有边框
		\value AllCorner 可调整所有角落
		\value All 可调整所有边框和角落
		\value RB 可调整右边框、下边框和右下角
	*/
	/*!
		\since Visindigo 0.13.0
		\a parent 安装该工具的父级QWidget。
		\a borders 指定哪些边框可以调整大小，默认是全部边框。
		\a borderWidth 指定边框的宽度，默认是5像素。
		\a topParent 指定事件过滤器安装的顶级父级QWidget，默认为parent。

		构造函数
		正确设置topParent有助于扩大鼠标跟踪范围，避免在调整窗口大小时鼠标离开边框区域导致调整中断。
		建议将topParent设置为窗口的顶级父级QWidget。如果不设置，则与parent相同。
	*/
	WidgetResizeTool::WidgetResizeTool(QWidget* parent, Borders borders, int borderWidth, QWidget* topParent) :QObject((QObject*)parent) {
		d = new WidgetResizeToolPrivate();
		d->EnabledBorders = borders;
		d->borderWidth = borderWidth;
		d->mouseChecker = new QTimer(this);
		d->mouseChecker->setInterval(50);
		
		if (topParent != nullptr) {
			d->topParent = topParent;
		}
		else {
			d->topParent = parent;
		}
		d->topParent->installEventFilter(this);
		d->topParent->setMouseTracking(true);

		connect(d->mouseChecker, &QTimer::timeout, this, [this]() {
			QPoint pos = d->topParent->mapFromGlobal(QCursor::pos());
			QRect rect = d->topParent->rect();
			if (!d->mouseInBorderArea(pos, rect)) {
				d->topParent->unsetCursor();
				d->resizing = false;
				d->mouseChecker->stop();
			}
			});
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	WidgetResizeTool::~WidgetResizeTool() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置哪些边框可以调整大小。
		\a borders 指定哪些边框可以调整大小，可以是Left、Right、Top、Bottom以及它们的组合。
	*/
	void WidgetResizeTool::setEnableBorder(Borders borders) {
		d->EnabledBorders = borders;
	}

	/*!
		\since Visindigo 0.13.0
		事件过滤器
		该函数会过滤安装了该工具的顶级父级QWidget的鼠标移动事件，以检测鼠标是否悬停在可调整大小的边框区域，并显示相应的调整大小光标。
		当用户按下鼠标左键并拖动时，该函数会根据鼠标移动的距离调整窗口的大小。
	*/
	bool WidgetResizeTool::eventFilter(QObject* target, QEvent* event) {
		if (target != d->topParent) {
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
						d->mouseChecker->start();
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