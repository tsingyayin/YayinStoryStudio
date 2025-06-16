#pragma once
#include "../Macro.h"

class QWidget;
namespace YSSCore::Widgets {
	class WidgetDisplayStackPrivate;
	class YSSCoreAPI WidgetDisplayStack {
	public:
		WidgetDisplayStack();
		bool pushWidget(QWidget* widget, bool deleteOnClose = false, bool ownerPtr = true);
		QWidget* popWidget();
	protected:
		WidgetDisplayStackPrivate* d;
	};
}