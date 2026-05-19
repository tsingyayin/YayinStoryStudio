#include "../DebugServerManager.h"
#include "../DebugServer.h"
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
namespace YSSCore::Editor {
	class DebugServerManagerPrivate {
		friend class DebugServerManager;
	protected:
		QMap<QString, DebugServer*> DebugServers;
		static DebugServerManager* Instance;
	};

	DebugServerManager* DebugServerManagerPrivate::Instance = nullptr;

	/*!
		\class YSSCore::Editor::DebugServerManager
		\inmodule YSSCore
		\brief 此类为Visindigo提供调试服务器管理器.
		\since YSS 0.13.0
		DebugServerManager负责管理调试服务器。
	*/

	/*!
		\fn void DebugServerManager::projectDebugServerChanged(const QString& serverID)
		\since YSS 0.15.0

		从0.15.0开始，当通过YSSCore::General::YSSProject::setProjectDebugServerID
		函数更改了当前项目的调试服务器时，DebugServerManager会发出这个信号。
		\a serverID 新设置的新调试服务器ID。

		由于YSSProject不想被设置为QObject子类，因此这个信号在这里提供。

		这个信号不保证serverID能够对应一个有效的DebugServer实例，因此在接收到这个信号时，
		应该调用getDebugServer函数来获取当前项目的调试服务器实例，并检查返回值是否为nullptr。

		此外，由于YSSProject的特性，这个信号不会在加载项目时发出，因此在加载项目后，
		应该主动调用getDebugServer函数来获取当前项目的调试服务器实例，只将该信号
		用于监听中途变更调试服务器的情况。
	*/

	/*!
		\since YSS 0.13.0
		构造函数
	*/
	DebugServerManager::DebugServerManager() {
		d = new DebugServerManagerPrivate();
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	DebugServerManager::~DebugServerManager() {
		delete d;
	}

	/*!
		\since YSS 0.13.0
		获取DebugServerManager实例的指针。
	*/
	DebugServerManager* DebugServerManager::getInstance() {
		if (DebugServerManagerPrivate::Instance == nullptr) {
			DebugServerManagerPrivate::Instance = new DebugServerManager();
		}
		return DebugServerManagerPrivate::Instance;
	}

	/*!
		\since YSS 0.13.0
		获取所有已注册的调试服务器ID列表。
	*/
	QStringList DebugServerManager::getDebugServerIDs() {
		return d->DebugServers.keys();
	}

	/*!
		\since YSS 0.13.0
		\a serverID 调试服务器ID
		根据ID获取调试服务器实例。
	*/
	DebugServer* DebugServerManager::getDebugServer(const QString& serverID) {
		if (d->DebugServers.contains(serverID)) {
			return d->DebugServers[serverID];
		}
		return nullptr;
	}

	/*!
		\since YSS 0.13.0
		\a server 调试服务器实例
		添加一个新的调试服务器实例。
	*/
	void DebugServerManager::addDebugServer(DebugServer* server) {
		if (server != nullptr) {
			d->DebugServers.insert(server->getModuleID(), server);
		}
	}
}