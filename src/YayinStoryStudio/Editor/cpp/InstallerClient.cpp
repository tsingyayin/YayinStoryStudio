#include "Editor/InstallerClient.h"
#include "General/Log.h"
#include <QtCore/qtimer.h>
namespace YSS::Editor {
	class InstallerClientPrivate {
		friend class InstallerClient;
	protected:
		QLocalSocket* socket;
		QTimer* timer;
		qint32 maxRetryCount = 5;
		qint32 retryCount = 0;
		QByteArray buffer;
		static InstallerClient* Instance;

		void handleReceivedData() {
			if (buffer.constData()[0] != 0x03) {
				socket->disconnectFromServer();
				vgErrorF << "YSS Installer Client received unknown data packet. Disconnecting...";
			}
			if (buffer.size() < 6) {
				return;
			}
			quint32 size = 0;
			memcpy(&size, buffer.constData() + 1, sizeof(size));
			if (buffer.size() < 6 + size) {
				return;
			}
			QString jsonData = QString::fromUtf8(buffer.constData() + 5, size);
			Visindigo::Utility::JsonConfig command = Visindigo::Utility::JsonConfig::fromJson(jsonData);
			handleCommand(command);
			if (buffer.constData()[size + 5] != 0x02) { // End of packet
				socket->disconnectFromServer();
				vgErrorF << "YSS Installer Client received unknown data packet. Disconnecting...";
			}
			buffer.remove(0, 6 + size);
		}

		void handleCommand(const Visindigo::Utility::JsonConfig& command) {
			if (command.contains("type")) {
				QString type = command.getString("type");
				if (type == "program_close") {
					emit InstallerClientPrivate::Instance->installerRequestProgramClose();
				}
			}
		}
	};

	InstallerClient* InstallerClientPrivate::Instance = nullptr;

	InstallerClient* InstallerClient::getInstance() {
		return InstallerClientPrivate::Instance;
	}

	InstallerClient::InstallerClient(QObject* parent) : QObject(parent) {
		d = new InstallerClientPrivate();
		InstallerClientPrivate::Instance = this;
		d->socket = new QLocalSocket(this);
		d->timer = new QTimer(this);
		d->timer->setInterval(1000);
		connect(d->timer, &QTimer::timeout, this, [this]() {
			if (d->socket->state() == QLocalSocket::ConnectedState) {
				d->timer->stop();
				emit connected();
			}
			else {
				if (d->socket->error() == QLocalSocket::ServerNotFoundError) {
					emit installerNotLaunched();
					d->timer->stop();
					return;
				}
				d->socket->abort();
				d->socket->connectToServer("YSSInstaller");
				d->retryCount++;
				if (d->retryCount >= d->maxRetryCount) {
					emit installerNotLaunched();
					d->timer->stop();
				}
			}
		});
		connect(d->socket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError socketError) {
			if (socketError == QLocalSocket::ServerNotFoundError) {
				emit installerNotLaunched();
			}
			else {
				qDebug() << "InstallerClient socket error: " << socketError;
			}
			});
		connect(d->socket, &QLocalSocket::disconnected, this, [this]() {
			emit disconnected();
			});
		connect(d->socket, &QLocalSocket::readyRead, this, [this]() {
			d->buffer.append(d->socket->readAll());
			while (d->socket->size() > 6) {
				d->handleReceivedData();
			}
		});
	}

	InstallerClient::~InstallerClient() {
		delete d;
		InstallerClientPrivate::Instance = nullptr;
	}

	void InstallerClient::connectToInstaller() {
		d->timer->start();
	}

	void InstallerClient::sendCommand(const Visindigo::Utility::JsonConfig& command) {
		if (d->socket->state() != QLocalSocket::ConnectedState) {
			vgErrorF << "YSS Installer Client is not connected to the installer. Cannot send command.";
			return;
		}
		QByteArray jsonData = command.toString().toUtf8();
		QByteArray dataPacket;
		dataPacket.append(0x03); // Start of packet
		quint32 size = jsonData.size();
		dataPacket.append(reinterpret_cast<const char*>(&size), sizeof(size));
		dataPacket.append(jsonData);
		dataPacket.append(0x02); // End of packet
		d->socket->write(dataPacket);
	}
}