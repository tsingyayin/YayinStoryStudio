#pragma once
#include <QtCore/QString>
#include "../Macro.h"

namespace YSSCore::Editor {
	class LangServer;
	class LangServerManagerPrivate;
	class YSSCoreAPI LangServerManager {
	public:
		LangServerManager();
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