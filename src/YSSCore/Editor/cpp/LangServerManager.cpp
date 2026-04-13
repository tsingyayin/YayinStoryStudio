#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <General/Log.h>
#include "../LangServerManager.h"
#include "../LangServer.h"
#include "General/YSSLogger.h"
namespace YSSCore::Editor {
	class LangServerManagerPrivate {
		friend class LangServerManager;
	protected:
		QList<LangServer*> LangServers;
		QMap<QString, LangServer*> IDServers;
		QMap<QString, LangServer*> ExtServers;
		static LangServerManager* Instance;
	};

	LangServerManager* LangServerManagerPrivate::Instance = nullptr;

	/*!
		\class YSSCore::Editor::LangServerManager
		\brief LangServerManager保存LangServer的实例
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup LangService

		LangServerManager是一个单例类，负责保存LangServer的实例。

		它真的只负责保存：如果调用removeServer，它不会自动销毁该实例。
	*/

	/*!
		\macro YSSLSM
		\since YSS 0.13.0
		\relates YSSCore::Editor::LangServerManager
		YSSCore::Editor::LangServerManager::getInstance()的简化写法
	*/

	/*!
		\since YSS 0.13.0
		构造函数
	*/
	LangServerManager::LangServerManager() {
		d = new LangServerManagerPrivate();
		ySuccessF << "Success!";
	}

	/*!
		\since YSS 0.13.0
		添加一个LangServer实例。如果ID或扩展名已经存在，则返回false。
	*/
	bool LangServerManager::addLangServer(LangServer* server) {
		QString ID = server->getLangID();
		QStringList Exts = server->getLangExts();
		if (d->IDServers.contains(ID)) {
			return false;
		}
		for (QString i : Exts) {
			if (d->ExtServers.contains(i)) {
				return false;
			}
		}
		d->IDServers.insert(ID, server);
		for (QString i : Exts) {
			d->ExtServers.insert(i, server);
		}
		d->LangServers.append(server);
		return true;
	}

	/*!
		\since YSS 0.13.0
		根据ID路由到一个LangServer实例。如果没有找到，则返回nullptr。
	*/
	LangServer* LangServerManager::routeID(const QString& id) {
		if (d->IDServers.contains(id)) {
			return d->IDServers.value(id);
		}
		return nullptr;
	}

	/*!
		\since YSS 0.13.0
		根据文件扩展名路由到一个LangServer实例。如果没有找到，则返回nullptr。
	*/
	LangServer* LangServerManager::routeExt(const QString& fileExt) {
		if (d->ExtServers.contains(fileExt)) {
			return d->ExtServers.value(fileExt);
		}
		return nullptr;
	}

	/*!
		\since YSS 0.13.0
		移除一个LangServer实例。如果没有找到，则不做任何事情。

		它不会顺带销毁该实例。
	*/
	void LangServerManager::removeServer(LangServer* server) {
		QString ID = server->getLangID();
		QStringList Exts = server->getLangExts();
		if (d->IDServers.contains(ID)) {
			d->IDServers.remove(ID);
		}
		for (QString i : Exts) {
			if (d->ExtServers.contains(i)) {
				d->ExtServers.remove(i);
			}
		}
		if (d->LangServers.contains(server)) {
			d->LangServers.removeOne(server);
		}
	}

	/*!
		\since YSS 0.13.0
		获取LangServerManager的实例。
	*/
	LangServerManager* LangServerManager::getInstance() {
		if (LangServerManagerPrivate::Instance == nullptr) {
			LangServerManagerPrivate::Instance = new LangServerManager();
		}
		return LangServerManagerPrivate::Instance;
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	LangServerManager::~LangServerManager() {
		delete d;
	}
}