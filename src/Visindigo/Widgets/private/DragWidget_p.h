#ifndef Visindigo_Widgets_DragWidget_p_h
#define Visindigo_Widgets_DragWidget_p_h

#include "Widgets/DragWidget.h"
#include <QtCore/qlist.h>
#include <QtWidgets/qlayout.h>
namespace Visindigo::Widgets {
	class DragWidget;
}
namespace Visindigo::__Private__ {
	class DragWidgetPrivate :public QObject{
		Q_OBJECT;
		friend class Visindigo::Widgets::DragWidget;
	protected:
		QWidget* q;
		QWidget* spaceWidget = nullptr;
		Qt::Orientation LayoutOrientation = Qt::Vertical;
		QList<QWidget*> Widgets;
		QLayout* Layout = nullptr;
		QPoint DragStartPos;
		QPoint DragDeltaPos;
		qint32 currentIndex = -1;
		QWidget* currentDraggingWidget = nullptr;
		bool dragging = false;
	public:
		virtual bool eventFilter(QObject* watched, QEvent* event) override;
		void onDragStart(QMouseEvent* event);
		void onDragEnd(QMouseEvent* event);
		void onDragMove(QMouseEvent* event);
		QWidget* getWidgetInLayout(QPoint localPos);
	};
}
#endif // Visindigo_Widgets_DragWidget_p_h