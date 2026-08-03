#include "Installer/InstallerServer.h"
#include "Installer/InstallerClientData.h"
#include <QtCore/qmap.h>
#include <General/Log.h>
#include <Utility/JsonConfig.h>
#include <QtCore/qset.h>
#include <QtWidgets/qapplication.h>
namespace YSS::Installer {
	struct InstallerClientSocket {
		QLocalSocket* socket;
		QByteArray buffer;
		InstallerClientData clientData;
	};

	class InstallerServerPrivate {
		friend class InstallerServer;
	protected:
		QLocalServer* server = nullptr;
		QLocalSocket* probeSocket = nullptr;
		QMap<QLocalSocket*, InstallerClientSocket> clientSockets;
		QSet<QLocalSocket*> activeSockets;
		static InstallerServer* Instance;

		void onSocketReadyRead(QLocalSocket* socket) {
			if (not activeSockets.contains(socket)) {
				activeSockets.insert(socket);
			}

			if (not clientSockets.contains(socket)) {
				return;
			}
			if (clientSockets[socket].buffer.constData()[0] != 0x03) {
				socket->disconnectFromServer();
				vgErrorF << "YSS Installer Server received unknown data packet. Disconnecting...";
				return;
			}
			if (clientSockets[socket].buffer.size() < 6) {
				return;
			}
			quint32 size = 0;
			memcpy(&size, clientSockets[socket].buffer.constData() + 1, sizeof(size));
			if (clientSockets[socket].buffer.size() < 6 + size) {
				return;
			}
			QString jsonData = QString::fromUtf8(clientSockets[socket].buffer.constData() + 5, size);
			Visindigo::Utility::JsonConfig command = Visindigo::Utility::JsonConfig::fromJson(jsonData);
			handleCommand(socket, command);
			if (clientSockets[socket].buffer.constData()[size + 5] != 0x02) { // End of packet
				socket->disconnectFromServer();
				vgErrorF << "YSS Installer Server received unknown data packet. Disconnecting...";
			}
			clientSockets[socket].buffer.remove(0, 6 + size);
		}

		void handleCommand(QLocalSocket* socket, const Visindigo::Utility::JsonConfig& command) {
			if (command.contains("type")) {
				QString type = command.getString("type");
				if (type == "client_data") {
					InstallerClientData clientData;
					Visindigo::Utility::JsonConfig dataConfig = command.getObject("data");
					clientData = Visindigo::Utility::JsonConfig::toMetable<InstallerClientData>(dataConfig);
					clientSockets[socket].clientData = clientData;
					vgDebug << "Received client data from socket:" << socket << 
						"Program Path:" << clientData.getProgramPath() << "Version:" << clientData.getProgramVersion() 
						<< "Auto Update Enabled:" << clientData.getAutoUpdateEnabled();
				}
			}
		}
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
		d->server = new QLocalServer(this);
		connect(d->probeSocket, &QLocalSocket::connected, this, [this]() {
			// Another installer instance is already running
			d->probeSocket->disconnectFromServer();
			emit installerHasLaunched();
		});
		connect(d->probeSocket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError error) {
			if (error != QLocalSocket::ServerNotFoundError)
				return;
			// No existing server — clean up probe and create our own
			d->server->listen("YSSInstaller");
			emit serverEstablished();
		});
		connect(d->server, &QLocalServer::newConnection, this, [this]() {
			QLocalSocket* clientSocket = d->server->nextPendingConnection();
			if (clientSocket) {
				emit clientConnected(clientSocket);
				d->clientSockets[clientSocket] = InstallerClientSocket{ clientSocket, QByteArray(), InstallerClientData() };
				connect(clientSocket, &QLocalSocket::readyRead, clientSocket, [this, clientSocket]() {
					d->clientSockets[clientSocket].buffer.append(clientSocket->readAll());
					d->onSocketReadyRead(clientSocket);
					});
				connect(clientSocket, &QLocalSocket::disconnected, clientSocket, [this, clientSocket]() {
					emit clientDisconnected(clientSocket);
					clientSocket->deleteLater();
					d->clientSockets.remove(clientSocket);
					if (d->activeSockets.contains(clientSocket)) {
						d->activeSockets.remove(clientSocket);
						if (d->activeSockets.isEmpty()) {
							qApp->quit();
						}
					}
				});
			}
		});
	}

	InstallerServer::~InstallerServer() {
		d->probeSocket->deleteLater();
		d->server->close();
		delete d;
		InstallerServerPrivate::Instance = nullptr;
	}

	void InstallerServer::launchServer() {
		d->probeSocket->connectToServer("YSSInstaller");
	}
}
