#include <QtCore/qdir.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtimer.h>
#include <QtWidgets/qapplication.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include <Utility/SevenZipBinder.h>
#include "Installer/InstallerClientData.h"
#include "Installer/InstallerServer.h"
#include "Installer/PSOD.h"
#include "Installer/TrayIcon.h"
#include "Installer/VersionManager.h"

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
		// 该 YSS 正在打包时不能退出安装程序
		bool isArchiving = false;
		// 崩溃页面显示期间也不能退出安装程序
		bool isCrashScreenShowing = false;
		static constexpr int CompressTimeoutMS = 60000;
		static InstallerServer* Instance;

		// 安装程序手上有活时就不能退出：升级、打包、显示崩溃页面
		bool isBusy() const {
			return isArchiving || isCrashScreenShowing || VersionManager::getInstance()->inUpdateProgress();
		}

		void onSocketReadyRead(QLocalSocket* socket) {
			if (not activeSockets.contains(socket)) {
				activeSockets.insert(socket);
			}

			if (not clientSockets.contains(socket)) {
				return;
			}
			// One readyRead() may deliver several packets, and handling a command can run a nested event
			// loop (archiving crash files waits on 7za), so keep draining the buffer until no complete
			// packet is left. The buffer is looked up again every round because handling a command may
			// disconnect and remove the socket.
			while (clientSockets.contains(socket)) {
				QByteArray& buffer = clientSockets[socket].buffer;
				if (buffer.size() < 6) { // start marker + 4-byte length + end marker
					return;
				}
				if (buffer.constData()[0] != 0x03) {
					socket->disconnectFromServer();
					vgErrorF << "YSS Installer Server received unknown data packet. Disconnecting...";
					return;
				}
				quint32 size = 0;
				memcpy(&size, buffer.constData() + 1, sizeof(size));
				const qsizetype packetSize = static_cast<qsizetype>(size) + 6;
				if (packetSize > buffer.size()) {
					return; // wait for the rest of the packet
				}
				if (buffer.constData()[packetSize - 1] != 0x02) { // End of packet
					socket->disconnectFromServer();
					vgErrorF << "YSS Installer Server received unknown data packet. Disconnecting...";
					return;
				}
				// Take the packet out of the buffer before dispatching it, so a nested readyRead that
				// runs during handling cannot consume the buffer underneath us.
				const QByteArray packet = buffer.left(packetSize);
				buffer.remove(0, packetSize);
				QString jsonData = QString::fromUtf8(packet.constData() + 5, size);
				Visindigo::Utility::JsonConfig command = Visindigo::Utility::JsonConfig::fromJson(jsonData);
				handleCommand(socket, command);
			}
		}

		void handleCommand(QLocalSocket* socket, const Visindigo::Utility::JsonConfig& command) {
			if (command.contains("type")) {
				QString type = command.getString("type");
				if (type == "client_data") {
					InstallerClientData clientData;
					Visindigo::Utility::JsonConfig dataConfig = command.getObject("data");
					clientData = Visindigo::Utility::JsonConfig::toMetable<InstallerClientData>(dataConfig);
					clientSockets[socket].clientData = clientData;
					VersionManager::getInstance()->recordYSSClient(clientData);
					vgDebug << "Received client data from socket:" << socket << 
						"Program Path:" << clientData.getProgramPath() << "Version:" << clientData.getProgramVersion() 
						<< "Auto Update Enabled:" << clientData.getAutoUpdateEnabled();
					TrayIcon::getInstance()->showMessage(VITRL("YSSInstaller::Monitor.Title"),  VITRL("YSSInstaller::Monitor.Launched").arg(clientData.getProgramVersion()));
				}
				else if (type == "program_close") {
					// 收到 program_close 命令时，关闭 YSSInstaller 自身。
					qApp->quit();
				}
				else if (type == "program_crashed") {
					// YSS 写完崩溃存证后发出，由安装程序负责把相关文件打包到该 YSS 的根目录
					archiveCrashFiles(socket);
				}
			}
		}

		void sendCommandTo(QLocalSocket* socket, const Visindigo::Utility::JsonConfig& command) {
			if (socket == nullptr || socket->state() != QLocalSocket::ConnectedState) {
				return;
			}
			// 与 InstallerClient::sendCommand 相同的二进制报文：0x03 起始 + 4 字节长度 + JSON 数据 + 0x02 结束。
			QByteArray jsonData = command.toString().toUtf8();
			QByteArray dataPacket;
			dataPacket.append(0x03); // Start of packet
			quint32 size = jsonData.size();
			dataPacket.append(reinterpret_cast<const char*>(&size), sizeof(size));
			dataPacket.append(jsonData);
			dataPacket.append(0x02); // End of packet
			socket->write(dataPacket);
		}

		// last_crash.txt 按行记录本次运行的日志、崩溃报告与转储。路径相对该 YSS 的根目录，
		// 取不到相对路径时写入的则是绝对路径。空行一概跳过，因此行序即日志/报告/转储。
		QStringList readCrashManifestLines(const QString& programDir) const {
			QStringList paths;
			QFile manifest(programDir + "/last_crash.txt");
			if (not manifest.open(QIODevice::ReadOnly)) {
				return paths;
			}
			QString content = QString::fromUtf8(manifest.readAll());
			// 用记事本改过的话文件开头会多一个 BOM，先去掉
			if (content.startsWith(QChar(0xFEFF))) {
				content.remove(0, 1);
			}
			manifest.close();
			for (const QString& rawLine : content.split(QLatin1Char('\n'))) {
				const QString line = rawLine.trimmed();
				if (line.isEmpty()) {
					continue;
				}
				paths << (QDir::isAbsolutePath(line) ? line : programDir + "/" + line);
			}
			return paths;
		}

		QStringList readCrashManifest(const QString& programDir) const {
			QStringList files;
			for (const QString& path : readCrashManifestLines(programDir)) {
				if (Visindigo::Utility::FileUtility::isFileExist(path)) {
					files << path;
				}
				else {
					vgWarning << "Crash file recorded in last_crash.txt does not exist:" << path;
				}
			}
			return files;
		}

		// 崩溃报告不可用时的兜底目录：与 CrashGateway 的默认布局一致
		QString fallbackReportFolder(const QString& programDir) const {
			return programDir + "/user_data/logs/crashreports";
		}

		// 启动 7za 并等它结束。7za 有文件没能处理时会返回非0，这里把真实结果报出来。
		bool runSevenZip(const QStringList& files, const QString& zipPath) {
			Visindigo::Utility::SevenZipBinder* binder = Visindigo::Utility::SevenZipBinder::getInstance();
			binder->bind7zaBinary(QCoreApplication::applicationDirPath() + "/7za.exe");
			if (not binder->isValid()) {
				vgErrorF << "7za.exe is not available, cannot archive crash files. Path:" << binder->get7zaBinaryPath();
				return false;
			}
			if (not binder->compressFilesToZip(files, zipPath, Visindigo::Utility::SevenZipBinder::zip)) {
				return false;
			}
			isArchiving = true;
			bool success = false;
			QEventLoop loop;
			QTimer timeoutTimer;
			timeoutTimer.setSingleShot(true);
			QMetaObject::Connection connection = QObject::connect(binder,
				&Visindigo::Utility::SevenZipBinder::processed, &loop, [&loop, &success](bool result) {
					success = result;
					loop.quit();
				});
			QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
			timeoutTimer.start(CompressTimeoutMS);
			loop.exec();
			QObject::disconnect(connection);
			isArchiving = false;
			if (not success) {
				vgWarning << "7za did not finish successfully for" << zipPath;
			}
			return success;
		}

		void archiveCrashFiles(QLocalSocket* socket) {
			const QString programPath = clientSockets.value(socket).clientData.getProgramPath();
			if (programPath.isEmpty()) {
				vgErrorF << "Received program_crashed from a socket without client data.";
				return;
			}
			const QString programDir = QFileInfo(programPath).absolutePath();
			const QString zipPath = programDir + "/last_crash.zip";
			// 日志此刻还被 YSS 自己占着，靠 SevenZipBinder 里那个 -ssw 才能压进去
			// （不加 -ssw 时 7za 会跳过被占用的文件，而且默认不报错）
			const QStringList files = readCrashManifest(programDir);
			if (files.isEmpty()) {
				vgWarning << "No crash file to archive for" << programPath;
			}
			else {
				// 7za 是往已有包里追加成员，先删掉上一次的包，避免新旧文件混在一起
				QFile::remove(zipPath);
				if (runSevenZip(files, zipPath)) {
					vgNotice << "Crash files of" << programPath << "archived to" << zipPath
						<< "(" << files.size() << "files)";
				}
			}
			// 压缩结束后再把崩溃页面交给用户，此刻崩溃产物已经齐了
			showCrashScreenLater(programDir);
			quitIfIdle();
		}

		// 崩溃页面推迟到本次 socket 处理返回之后再显示：在 readyRead 里再套一层模态循环，
		// 容易和协议解析、断开处理互相嵌套。占位标记必须在这里同步置上，
		// 否则处理函数返回后 YSS 一旦断开，退出逻辑就会抢在页面显示之前把安装程序关掉。
		void showCrashScreenLater(const QString& programDir) {
			isCrashScreenShowing = true;
			QTimer::singleShot(0, InstallerServer::getInstance(), [this, programDir]() {
				showCrashScreen(programDir);
				});
		}

		// 页面内容全部取自落盘文件：报告路径来自 last_crash.txt 的第二行。
		void showCrashScreen(const QString& programDir) {
			const QStringList manifest = readCrashManifestLines(programDir);
			const QString reportPath = manifest.size() > 1 ? manifest.at(1) : QString();
			PSOD psod(reportPath, fallbackReportFolder(programDir));
			psod.execute();
			isCrashScreenShowing = false;
			quitIfIdle();
		}

		void quitIfIdle() {
			if (activeSockets.isEmpty() and not isBusy()) {
				qApp->quit();
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
							// 手上还有活时把退出推后，等活干完再退
							if (not d->isBusy()) {
								qApp->quit();
							}
							else {
								emit allClientDisconnected();
							}
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

	qint32 InstallerServer::getConnectedClientCount() {
		return d->clientSockets.size();
	}

	QLocalSocket* InstallerServer::getSocketByClientData(const InstallerClientData& clientData) const {
		for (auto it = d->clientSockets.constBegin(); it != d->clientSockets.constEnd(); ++it) {
			if (it.value().clientData.getProgramPath() == clientData.getProgramPath()) {
				return it.key();
			}
		}
		return nullptr;
	}

	void InstallerServer::sendCommand(const InstallerClientData& clientData, const Visindigo::Utility::JsonConfig& command) {
		QLocalSocket* socket = getSocketByClientData(clientData);
		if (socket == nullptr || socket->state() != QLocalSocket::ConnectedState) {
			vgErrorF << "YSS Installer Server cannot send command: client socket not found or not connected.";
			return;
		}
		d->sendCommandTo(socket, command);
	}

	void InstallerServer::sendUpdateYSSInstallerRequest(const InstallerClientData& clientData) {
		Visindigo::Utility::JsonConfig command;
		command.setString("type", "update_yss_installer");
		sendCommand(clientData, command);
	}

	bool InstallerServer::isClientStillRunning(const InstallerClientData& clientData) {
		QLocalSocket* socket = getSocketByClientData(clientData);
		return socket != nullptr && socket->state() == QLocalSocket::ConnectedState;
	}
}
