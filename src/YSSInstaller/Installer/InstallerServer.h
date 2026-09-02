#ifndef YSSInstaller_Installer_InstallerServer_h
#define YSSInstaller_Installer_InstallerServer_h
#include <QtCore/qobject.h>
#include <QtNetwork/qlocalserver.h>
#include <QtNetwork/qlocalsocket.h>
#include <General/Version.h>
#include "Installer/InstallerClientData.h"
namespace Visindigo::Utility { class JsonConfig; }
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
		QLocalSocket* getSocketByClientData(const InstallerClientData& clientData) const;
		void sendCommand(const InstallerClientData& clientData, const Visindigo::Utility::JsonConfig& command);
		void sendUpdateYSSInstallerRequest(const InstallerClientData& clientData);
		bool isClientStillRunning(const InstallerClientData& clientData);
	private:
		InstallerServerPrivate* d;
	};
}

#endif // YSSInstaller_Installer_InstallerServer_h
