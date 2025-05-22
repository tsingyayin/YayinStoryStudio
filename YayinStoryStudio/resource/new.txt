#include "../LangServerManager.h"
#include "../LangServer.h"
#include <Windows.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
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

	LangServerManager::LangServerManager() {
		LangServerManagerPrivate::Instance = this;
		d = new LangServerManagerPrivate();
	}

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

	LangServer* LangServerManager::routeID(const QString& id) {
		if (d->IDServers.contains(id)) {
			return d->IDServers.value(id);
		}
		return nullptr;
	}
	LangServer* LangServerManager::routeExt(const QString& fileExt) {
		if (d->ExtServers.contains(fileExt)) {
			return d->ExtServers.value(fileExt);
		}
		return nullptr;
	}
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
	LangServerManager* LangServerManager::getInstance() {
		return LangServerManagerPrivate::Instance;
	}
	LangServerManager::~LangServerManager() {
		delete d;
	}
}