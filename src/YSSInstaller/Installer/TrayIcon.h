#ifndef YSS_Installer_TrayIcon_h
#define YSS_Installer_TrayIcon_h
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
#endif // YSS_Installer_TrayIcon_h
