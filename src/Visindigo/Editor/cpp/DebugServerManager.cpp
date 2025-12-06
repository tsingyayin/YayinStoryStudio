#include "../DebugServerManager.h"
#include "../DebugServer.h"
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
namespace Visindigo::Editor {
	class DebugServerManagerPrivate {
		friend class DebugServerManager;
	protected:
		QMap<QString, DebugServer*> DebugServers;
		static DebugServerManager* Instance;
	};

	DebugServerManager* DebugServerManagerPrivate::Instance = nullptr;

	/*!
		\class Visindigo::Editor::DebugServerManager
		\inmodule Visindigo
		\brief 此类为Visindigo提供调试服务器管理器。
		\since Visindigo 0.13.0
		DebugServerManager负责管理调试服务器。
	*/
	DebugServerManager::DebugServerManager() {
		d = new DebugServerManagerPrivate();
	}

	DebugServerManager::~DebugServerManager() {
		delete d;
	}

	DebugServerManager* DebugServerManager::getInstance() {
		if (DebugServerManagerPrivate::Instance == nullptr) {
			DebugServerManagerPrivate::Instance = new DebugServerManager();
		}
		return DebugServerManagerPrivate::Instance;
	}

	DebugServer* DebugServerManager::getDebugServer(const QString& serverID) {
		if (d->DebugServers.contains(serverID)) {
			return d->DebugServers[serverID];
		}
		return nullptr;
	}

	void DebugServerManager::addDebugServer(DebugServer* server) {
		if (server != nullptr) {
			d->DebugServers.insert(server->getModuleID(), server);
		}
	}
}