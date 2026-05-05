#ifndef Visindigo_Widgets_DragWidget_h
#define Visindigo_Widgets_DragWidget_h
#include "VICompileMacro.h"
#include <QtWidgets/qframe.h>
class QBoxLayout;
namespace Visindigo::__Private__ {
	class DragWidgetPrivate;
}
namespace Visindigo::Widgets {
	class VisindigoAPI DragWidget :public QFrame {
		Q_OBJECT
	public:
		DragWidget(QWidget* parent = nullptr);
		virtual ~DragWidget();
	public:
		void setLayoutPolicy(Qt::Orientation orientation);
		void addWidget(QWidget* widget);
		void removeWidget(QWidget* widget);
		QBoxLayout* getBoxLayout() const;
	private:
		Visindigo::__Private__::DragWidgetPrivate* d;
	};
}

#endif // Visindigo_Widgets_DragWidget_h
