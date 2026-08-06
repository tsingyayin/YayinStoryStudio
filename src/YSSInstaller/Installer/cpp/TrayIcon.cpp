#include "Installer/TrayIcon.h"
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmessagebox.h>
#include <General/TranslationHost.h>
namespace YSS::Installer {
	class TrayIconPrivate {
		friend class TrayIcon;
	protected:
		QMenu* menu;
		QAction* aboutAction;
		static TrayIcon* instance;
	};

	TrayIcon* TrayIconPrivate::instance = nullptr;

	TrayIcon::TrayIcon() {
		TrayIconPrivate::instance = this;
		setIcon(QIcon(":/resource/cn.yxgeneral.yss_installer/icon.png"));
		d = new TrayIconPrivate();
		d->menu = new QMenu();
		d->aboutAction = d->menu->addAction(VITR("YSSInstaller::TrayIcon.About"));
		connect(d->aboutAction, &QAction::triggered, this, [this]() {
			QMessageBox::about(nullptr, VITR("YSSInstaller::About.Title"), VITR("YSSInstaller::About.Desc"));
			});
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