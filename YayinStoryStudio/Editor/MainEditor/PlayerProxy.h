#pragma once
#include <QObject>

class QWindow;
class QWidget;

namespace YSS::Editor {
	class PlayerProxy :public QObject {
		Q_OBJECT;
	private:
		QWindow* ProxyWindow = nullptr;
		QWidget* ProxyWidget = nullptr;
	public:
		PlayerProxy(QWidget* parent = nullptr);
		QWidget* doASEProxy();
	};
}