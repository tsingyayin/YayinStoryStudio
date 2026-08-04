#ifndef YSS_Installer_InstallerServer_h
#define YSS_Installer_InstallerServer_h
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
		//! 根据客户端数据（程序路径）索引其对应的 Socket；未找到返回 nullptr。
		QLocalSocket* getSocketByClientData(const InstallerClientData& clientData) const;
		//! 通用命令发送函数：向 \a clientData 对应的客户端发送二进制命令报文，供日后扩展使用。
		void sendCommand(const InstallerClientData& clientData, const Visindigo::Utility::JsonConfig& command);
		//! 向指定客户端发送“更新 YSS 安装程序自身”的请求。
		void sendUpdateYSSInstallerRequest(const InstallerClientData& clientData);
	private:
		InstallerServerPrivate* d;
	};
}

#endif // YSS_Installer_InstallerServer_h
