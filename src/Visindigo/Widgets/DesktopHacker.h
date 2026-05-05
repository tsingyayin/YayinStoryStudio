#ifndef Visindigo_Widgets_DesktopHacker_h
#define Visindigo_Widgets_DesktopHacker_h
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

#endif // Visindigo_Widgets_DesktopHacker_h