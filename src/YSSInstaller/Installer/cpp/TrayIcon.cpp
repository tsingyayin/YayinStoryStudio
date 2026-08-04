#include "Installer/TrayIcon.h"
#include <QtWidgets/qmenu.h>
namespace YSS::Installer {
	class TrayIconPrivate {
		friend class TrayIcon;
	protected:
		QMenu* menu;
		QAction* exitAction;
		static TrayIcon* instance;
	};

	TrayIcon* TrayIconPrivate::instance = nullptr;

	TrayIcon::TrayIcon() {
		TrayIconPrivate::instance = this;
		setIcon(QIcon(":/resource/cn.yxgeneral.yss_installer/icon.png"));
		d = new TrayIconPrivate();
		d->menu = new QMenu();
		d->exitAction = d->menu->addAction("Exit");
		setContextMenu(d->menu);
		setToolTip("YSS Installer");
	}

	TrayIcon::~TrayIcon() {
		TrayIconPrivate::instance = nullptr;
		delete d;
	}

	TrayIcon* TrayIcon::getInstance() {
		return TrayIconPrivate::instance;
	}
}