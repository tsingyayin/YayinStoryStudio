#pragma once
#include "../Macro.h"
// Forward declarations
class QWidget;
namespace Visindigo::Widgets {
	class WidgetDisplayStackPrivate;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI WidgetDisplayStack {
	public:
		WidgetDisplayStack();
		bool pushWidget(QWidget* widget, bool deleteOnClose = false, bool ownerPtr = true);
		QWidget* popWidget();
	protected:
		WidgetDisplayStackPrivate* d;
	};
}