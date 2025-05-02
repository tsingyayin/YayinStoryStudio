#include "../PlayerProxy.h"
#include <Windows.h>

namespace YSS::Editor {
	PlayerProxy::PlayerProxy(QWidget* parent) : QObject(parent) {
		
	}

	QWidget* PlayerProxy::doASEProxy() {
		HWND hwnd = FindWindow(nullptr, L"Arknights_StoryEditor");
		if (hwnd == nullptr) {
			return nullptr;
		}
		if (ProxyWindow != nullptr) {
			delete ProxyWindow;
		}
		if (ProxyWidget != nullptr) {
			delete ProxyWidget;
		}
		//get raw width and height
		RECT rect;
		GetWindowRect(hwnd, &rect);
		ProxyWindow = QWindow::fromWinId((WId)hwnd);
		ProxyWindow->showFullScreen();
		ProxyWidget = QWidget::createWindowContainer(ProxyWindow, (QWidget*)parent(), Qt::Widget);
		ProxyWidget->setFixedSize(1920, 1080);
		return ProxyWidget;
	}
}