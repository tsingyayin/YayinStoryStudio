#ifndef Visindigo_Widgets_BorderFrame_h
#define Visindigo_Widgets_BorderFrame_h
#include <QtWidgets/qframe.h>
#include "VICompileMacro.h"
namespace Visindigo::Widgets {
	class VisindigoAPI BorderFrame :public QFrame {
		Q_OBJECT;
	public:
		BorderFrame(QWidget* parent = nullptr);
		virtual ~BorderFrame();
	public:
		virtual void paintEvent(QPaintEvent* event) override;
	};
}
#endif Visindigo_Widgets_BorderFrame_h