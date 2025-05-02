#pragma once

class QWidget;
namespace YSS::Editor {
	class StackWidget {
	public:
		virtual bool onWidgetClose() = 0;
		virtual QWidget* getWidget() = 0;
	};
}