#ifndef YSSInstaller_Installer_TrayIcon_h
#define YSSInstaller_Installer_TrayIcon_h
#include <QtCore/qobject.h>
#include <QtWidgets/qsystemtrayicon.h>
namespace YSS::Installer {
	class TrayIconPrivate;
	class TrayIcon :public QSystemTrayIcon {
		Q_OBJECT;
	public:
		TrayIcon();
		virtual ~TrayIcon();
		static TrayIcon* getInstance();
	private:
		TrayIconPrivate* d;
	};
}
#endif // YSSInstaller_Installer_TrayIcon_h
