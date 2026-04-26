#include "Widgets/DesktopHacker.h"
#include <QtGui/qwindow.h>
#include <QtWidgets/qwidget.h>
#include <QtGui/qguiapplication.h>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif 

namespace Visindigo::Widgets {
	class DesktopHackerPrivate {
		friend class DesktopHacker;
	protected:
		WId DesktopWinId = 0;
		QWidget* SuspendedWidget = nullptr;
		static DesktopHacker* Instance;
		void initDesktop() {
			PDWORD_PTR result = NULL;
			SendMessageTimeout(FindWindow("Progman", NULL), 0x52C, 0, 0, SMTO_NORMAL, 1000, result);
			HWND WorkerW = NULL;
			HWND DefView = NULL;
			WorkerW = FindWindowEx(NULL, NULL, "WorkerW", NULL);
			while((not DefView) && WorkerW) {
				DefView = FindWindowEx(WorkerW, NULL, "SHELLDLL_DefView", NULL);
				WorkerW = FindWindowEx(NULL, WorkerW, "WorkerW", NULL);
			}
			ShowWindow(WorkerW, 0);
			HWND desktop = FindWindow("Progman", NULL);
			DesktopWinId = (WId)desktop;
		}
		void restoreDesktop() {
			ShowWindow((HWND)DesktopWinId, 1);
		}
		void suspendWidget(QWidget* widget) {
			if (SuspendedWidget) {
				return;
			}
			widget->setParent(nullptr);
			widget->winId();
			SetParent((HWND)widget->winId(), (HWND)DesktopWinId);
			SuspendedWidget = widget;
		}
		QWidget* popWidget() {
			if (not SuspendedWidget) {
				return nullptr;
			}
			SetParent((HWND)SuspendedWidget->winId(), NULL);
			QWidget* result = SuspendedWidget;
			SuspendedWidget = nullptr;
			return result;
		}
		void suspendWindow(QWindow* window) {
			if (SuspendedWidget) {
				return;
			}
			window->setParent(nullptr);
			window->winId();
			SetParent((HWND)window->winId(), (HWND)DesktopWinId);
		}
		~DesktopHackerPrivate() {
			if (SuspendedWidget) {
				popWidget();
			}
			restoreDesktop();
			Instance = nullptr;
		}
	};

	DesktopHacker* DesktopHackerPrivate::Instance = nullptr;

	DesktopHacker* DesktopHacker::getInstance() {
		if (DesktopHackerPrivate::Instance == nullptr) {
			DesktopHackerPrivate::Instance = new DesktopHacker();
		}
		return DesktopHackerPrivate::Instance;
	}

	DesktopHacker::DesktopHacker() {
		d = new DesktopHackerPrivate();
		d->initDesktop();
	}

	DesktopHacker::~DesktopHacker() {
		delete d;
	}

	void DesktopHacker::suspendQWidget(QWidget* widget) {
		d->suspendWidget(widget);
	}

	void DesktopHacker::suspendWindow(QWindow* window) {
		d->suspendWindow(window);
	}
}