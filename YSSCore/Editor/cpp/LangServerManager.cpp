#include "../LangServerManager.h"
#include "../LangServer.h"
#include <Windows.h>
namespace YSSCore::Editor {
	class LangServerManagerPrivate {
		friend class LangServerManager;
	protected:
		QList<LangServer*> LangServers;
		QMap<QString, LangServer*> IDServers;
		QMap<QString, LangServer*> ExtServers;
		QList<HMODULE> dlls;
		static QFileInfoList recursionGetAllDll(const QString& path) {
			QDir dir(path);
			QStringList filters;
			filters << "*.yssls";
			dir.setNameFilters(filters);
			QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
			dir = QDir(path);
			QFileInfoList subList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
			for (int i = 0; i < subList.size(); i++) {
				qDebug() << "scan sub dir:" << subList[i].absoluteFilePath();
				list.append(recursionGetAllDll(subList[i].absoluteFilePath()));
			}
			return list;
		}
	};

	LangServerManager::LangServerManager(QObject* parent):QObject(parent) {
		p = new LangServerManagerPrivate();
		scan();
	}

	void LangServerManager::scan() {
		if (p->dlls.size() > 0) {
			for (int i = 0; i < p->dlls.size(); i++) {
				FreeLibrary(p->dlls[i]);
			}
			p->dlls.clear();
			for (int i = 0; i < p->LangServers.size(); i++) {
				p->LangServers[i]->serverDisable();
				delete p->LangServers[i];
			}
			p->LangServers.clear();
		}
		QFileInfoList list = LangServerManagerPrivate::recursionGetAllDll("./resource/server");
		for (int i = 0; i < list.size(); i++) {
			QFileInfo fileInfo = list.at(i);
			qDebug() << "LangServerManager: load dll:" << fileInfo.absoluteFilePath();
			QString fileName = fileInfo.fileName();
			QString filePath = fileInfo.absoluteFilePath();
			QString fileFolder = fileInfo.absolutePath();
			HMODULE hModule = LoadLibraryA(filePath.toStdString().c_str());
			if (hModule == nullptr) {
				qDebug() << "LangServerManager: load dll failed!";
				continue;
			}
			else {
				__YSSLangServerDllMain LangServerDllMain = (__YSSLangServerDllMain)(GetProcAddress(hModule, "YSSLangServerDllMain"));
				if (LangServerDllMain == nullptr) {
					qDebug() << "LangServerManager: get dll entry point failed!";
					FreeLibrary(hModule);
					continue;
				}
				LangServer* server = LangServerDllMain();
				if (server == nullptr) {
					qDebug() << "LangServerManager: create LangServer failed!";
					FreeLibrary(hModule);
					continue;
				}
				server->setPluginFolder(fileFolder);
				server->serverEnable();
				if (addLangServer(server)) {
					p->dlls.append(hModule);
					qDebug() << "LangServerManager:" << server->getLangID() << " LangServer loaded!";
				}
				else {
					qDebug() << "LangServerManager: add LangServer failed!";
					server->serverDisable();
					delete server;
					FreeLibrary(hModule);
				}
			}
		}
	}
	bool LangServerManager::addLangServer(LangServer* server) {
		QString ID = server->getLangID();
		QStringList Exts = server->getLangExts();
		if (p->IDServers.contains(ID)) {
			return false;
		}
		for (QString i : Exts) {
			if (p->ExtServers.contains(i)) {
				return false;
			}
		}
		p->IDServers.insert(ID, server);
		for (QString i : Exts) {
			p->ExtServers.insert(i, server);
		}
		p->LangServers.append(server);
		return true;
	}

	LangServer* LangServerManager::routeID(const QString& id) {
		if (p->IDServers.contains(id)) {
			return p->IDServers.value(id);
		}
		return nullptr;
	}
	LangServer* LangServerManager::routeExt(const QString& fileExt) {
		if (p->ExtServers.contains(fileExt)) {
			return p->ExtServers.value(fileExt);
		}
		return nullptr;
	}
	void LangServerManager::removeServer(LangServer* server) {
		QString ID = server->getLangID();
		QStringList Exts = server->getLangExts();
		if (p->IDServers.contains(ID)) {
			p->IDServers.remove(ID);
		}
		for (QString i : Exts) {
			if (p->ExtServers.contains(i)) {
				p->ExtServers.remove(i);
			}
		}
		if (p->LangServers.contains(server)) {
			p->LangServers.removeOne(server);
		}
	}
	LangServerManager::~LangServerManager() {
		delete p;
	}
}