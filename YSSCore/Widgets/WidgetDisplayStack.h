#pragma once
#include "../Macro.h"
// Forward declarations
class QWidget;
namespace YSSCore::Widgets {
	class WidgetDisplayStackPrivate;
}
// Main
namespace YSSCore::Widgets {
	class YSSCoreAPI WidgetDisplayStack {
	public:
		WidgetDisplayStack();
		bool pushWidget(QWidget* widget, bool deleteOnClose = false, bool ownerPtr = true);
		QWidget* popWidget();
	protected:
		WidgetDisplayStackPrivate* d;
	};
}