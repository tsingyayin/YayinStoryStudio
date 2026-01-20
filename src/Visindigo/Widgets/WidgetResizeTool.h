#ifndef Visindigo_Widgets_WidgetResizeTool_h
#define Visindigo_Widgets_WidgetResizeTool_h
#include "../Macro.h"
#include <QtCore/qobject.h>
#include <QtWidgets/qwidget.h>
namespace Visindigo::Widgets {
	class WidgetResizeToolPrivate;
	class VisindigoAPI WidgetResizeTool :public QObject {
		Q_OBJECT;
	public:
		enum Border {
			Left = 0x01,
			Top = 0x02,
			Right = 0x04,
			Bottom = 0x08,
			LeftTop = 0x10,
			RightTop = 0x20,
			LeftBottom = 0x40,
			RightBottom = 0x80,
			AllBorder = Left | Top | Right | Bottom,
			AllCorner = LeftTop | RightTop | LeftBottom | RightBottom,
			All = AllBorder | AllCorner,
			RB = Right | Bottom | RightBottom,
		};
		Q_DECLARE_FLAGS(Borders, Border);
	public:
		WidgetResizeTool(QWidget* parent, Borders borders = All, int borderWidth = 5);
		~WidgetResizeTool();
		void setEnableBorder(Borders borders);
		virtual bool eventFilter(QObject* target, QEvent* event) override;
	private:
		WidgetResizeToolPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(WidgetResizeTool::Borders);
}
#endif // Visindigo_Widgets_WidgetResizeTool_h