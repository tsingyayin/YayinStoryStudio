#ifndef YSS_Installer_InstallerServer_h
#define YSS_Installer_InstallerServer_h
#include <QtCore/qobject.h>
#include <QtNetwork/qlocalserver.h>
#include <QtNetwork/qlocalsocket.h>
#include <General/Version.h>
namespace YSS::Installer {
	class InstallerServerPrivate;
	class InstallerServer : public QObject {
		Q_OBJECT;
	signals:
		void clientConnected(QLocalSocket* client);
		void clientDisconnected(QLocalSocket* client);
		void installerHasLaunched();
		void serverEstablished();
		void allClientDisconnected();
	public:
		static InstallerServer* getInstance();
	public:
		InstallerServer(QObject* parent = nullptr);
		virtual ~InstallerServer();
		void launchServer();
		qint32 getConnectedClientCount();
	private:
		InstallerServerPrivate* d;
	};
}

#endif // YSS_Installer_InstallerServer_h
