#pragma once
#include "VICompileMacro.h"
#include <QtWidgets/qwidget.h>

namespace Visindigo::Widgets {
	class DesktopHackerPrivate;
	class VisindigoAPI DesktopHacker {
	private:
		DesktopHacker();
	public:
		static DesktopHacker* getInstance();
		~DesktopHacker();
		void suspendQWidget(QWidget* widget);
		void suspendWindow(QWindow* window);
		QWidget* popQWidget();
	private:
		DesktopHackerPrivate* d;
	};
}