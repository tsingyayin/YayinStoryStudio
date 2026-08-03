#include "Installer/InstallerServer.h"

namespace YSS::Installer {
	class InstallerServerPrivate {
		friend class InstallerServer;
	protected:
		QLocalServer* server = nullptr;
		QLocalSocket* probeSocket = nullptr;
		static InstallerServer* Instance;
	};

	InstallerServer* InstallerServerPrivate::Instance = nullptr;

	InstallerServer* InstallerServer::getInstance() {
		return InstallerServerPrivate::Instance;
	}

	InstallerServer::InstallerServer(QObject* parent) : QObject(parent) {
		d = new InstallerServerPrivate();
		InstallerServerPrivate::Instance = this;

		// Async probe: check if "YSSInstaller" server already exists
		d->probeSocket = new QLocalSocket(this);
		connect(d->probeSocket, &QLocalSocket::connected, this, [this]() {
			// Another installer instance is already running
			d->probeSocket->disconnectFromServer();
			d->probeSocket->deleteLater();
			d->probeSocket = nullptr;
			emit installerHasLaunched();
		});
		connect(d->probeSocket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError error) {
			if (error != QLocalSocket::ServerNotFoundError)
				return;
			// No existing server — clean up probe and create our own
			d->probeSocket->deleteLater();
			d->probeSocket = nullptr;

			d->server = new QLocalServer(this);
			connect(d->server, &QLocalServer::newConnection, this, [this]() {
				QLocalSocket* clientSocket = d->server->nextPendingConnection();
				if (clientSocket) {
					emit clientConnected(clientSocket);
					connect(clientSocket, &QLocalSocket::disconnected, this, [this, clientSocket]() {
						emit clientDisconnected(clientSocket);
						clientSocket->deleteLater();
					});
				}
			});
			d->server->listen("YSSInstaller");
			emit serverEstablished();
		});
		d->probeSocket->connectToServer("YSSInstaller");
	}

	InstallerServer::~InstallerServer() {
		if (d->probeSocket) {
			d->probeSocket->deleteLater();
		}
		if (d->server) {
			d->server->close();
		}
		delete d;
		InstallerServerPrivate::Instance = nullptr;
	}
}
