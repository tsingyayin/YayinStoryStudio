#ifndef Visindigo_Widgets_BorderLabel_h
#define Visindigo_Widgets_BorderLabel_h
#include <QtWidgets/qlabel.h>
#include "VICompileMacro.h"
namespace Visindigo::Widgets {
	class VisindigoAPI BorderLabel :public QLabel {
		Q_OBJECT;
	public:
		BorderLabel(QWidget* parent = nullptr);
		virtual ~BorderLabel();
	public:
		virtual void paintEvent(QPaintEvent* event) override;
	};
}
#endif Visindigo_Widgets_BorderLabel_h