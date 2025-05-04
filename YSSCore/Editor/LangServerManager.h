#pragma once
#include <QtCore>
#include <QtGui>
#include "../Macro.h"

namespace YSSCore::Editor {
	class LangServer;
	class LangServerManagerPrivate;
	class YSSCoreAPI LangServerManager :public QObject{
		Q_OBJECT;
	public:
		LangServerManager(QObject* parent = nullptr);
		~LangServerManager();
		bool addLangServer(LangServer* server);
		LangServer* routeID(const QString& id);
		LangServer* routeExt(const QString& fileExt);
		void removeServer(LangServer* server);
		static LangServerManager* getInstance();
	private:
		LangServerManagerPrivate* p;
	};
}

#define YSSLSM YSSCore::Editor::LangServerManager::getInstance()