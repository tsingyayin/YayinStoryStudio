#include "Editor/InstallerClient.h"
#include "General/Log.h"
#include <QtCore/qtimer.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstringlist.h>
#include <General/VIApplication.h>
#include <General/Plugin.h>
#include <General/TranslationHost.h>
#include <Utility/FileUtility.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qprocess.h>
#include "Utility/Console.h"
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
				else if (type == "update_yss_installer") {
					Visindigo::Utility::JsonConfig closeCommand;
					closeCommand.setString("type", "program_close");
					InstallerClientPrivate::Instance->sendCommand(closeCommand);
					InstallerClient::releaseInstaller(true, true);
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
		d->timer->setInterval(5000);
		connect(this, &YSS::Editor::InstallerClient::installerNotLaunched, this, [this]() {
			QString installerPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
				"/AppData/Local/TsingYayin/YayinStoryStudio/Installer/YSSInstaller.exe";
			if (not QProcess::startDetached(installerPath)) {
				vgErrorF << "Failed to launch installer: " << installerPath;
			}
			QTimer::singleShot(5000, this, [this]() {
				YSS::Editor::InstallerClient::getInstance()->connectToInstaller();
				});
			});
		connect(d->timer, &QTimer::timeout, this, [this]() {
			if (d->socket->state() == QLocalSocket::ConnectedState) {
				d->timer->stop();
			}
			else {
				d->socket->abort();
				d->socket->connectToServer("YSSInstaller");
				d->retryCount++;
				if (d->retryCount >= d->maxRetryCount) {
					d->retryCount = 0;
					vgError << "YSS Installer Client failed to connect to installer after" << d->maxRetryCount << "attempts.";
					d->timer->stop();
				}
			}
		});
		connect(d->socket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError socketError) {
			if (socketError == QLocalSocket::ServerNotFoundError) {
				emit installerNotLaunched();
			}
			else {
				vgError << "InstallerClient socket error: " << socketError;
			}
			});
		connect(d->socket, &QLocalSocket::disconnected, this, [this]() {
			emit disconnected();
			});
		connect(d->socket, &QLocalSocket::readyRead, this, [this]() {
			d->buffer.append(d->socket->readAll());
			d->handleReceivedData();
		});
		connect(d->socket, &QLocalSocket::connected, this, [this]() {
			d->retryCount = 0;
			d->timer->stop();
			emit connected();
			Visindigo::Utility::JsonConfig command;
			command.setString("type", "client_data");
			command.setString("data.programPath", QCoreApplication::applicationFilePath());
			command.setString("data.programVersion", VIApp->getMainPlugin()->getPluginVersion().toString());
			command.setBool("data.autoUpdateEnabled", true);
			sendCommand(command);
			});
	}

	InstallerClient::~InstallerClient() {
		delete d;
		InstallerClientPrivate::Instance = nullptr;
	}

	void InstallerClient::connectToInstaller() {
		d->socket->connectToServer("YSSInstaller");
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

	void InstallerClient::releaseInstaller(bool autoLaunch, bool showNotification) {
		if (showNotification) {
			QMessageBox::information(nullptr,
				VITRL("YSS::update.installerUpdating"),
				VITRL("YSS::update.installerUpdatingDesc"));
		}
		QString installerFolder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
			"/AppData/Local/TsingYayin/YayinStoryStudio/Installer";
		QStringList files = {
			"Visindigo.dll", "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6Network.dll", "Qt6Sql.dll",
			"Qt6Svg.dll", "dbghelp.dll", "icuuc.dll", "opengl32sw.dll", "7za.exe", "YSSInstaller.exe"
		};
		QStringList folders = {
			"iconengiens", "imageformats", "networkinformation", "platforms", "styles", "translations"
		};
		Visindigo::Utility::FileUtility::createDir(installerFolder);
		for (const QString& file : files) {
			Visindigo::Utility::FileUtility::copyFile(Visindigo::Utility::FileUtility::getProgramPath() +
				"/" + file, installerFolder + "/" + file, false, true);
		}
		for (const QString& folder : folders) {
			Visindigo::Utility::FileUtility::copyDir(Visindigo::Utility::FileUtility::getProgramPath() +
				"/" + folder, installerFolder + "/" + folder, false, true);
		}
		if (autoLaunch) {
			getInstance()->connectToInstaller();
		}
	}
}