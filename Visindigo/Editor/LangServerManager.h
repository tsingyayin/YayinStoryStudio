#pragma once
#include "../Macro.h"

class QString;
namespace Visindigo::Editor {
	class LangServer;
	class LangServerManagerPrivate;
	class VisindigoAPI LangServerManager {
	public:
		LangServerManager();
		~LangServerManager();
		bool addLangServer(LangServer* server);
		LangServer* routeID(const QString& id);
		LangServer* routeExt(const QString& fileExt);
		void removeServer(LangServer* server);
		static LangServerManager* getInstance();
	private:
		LangServerManagerPrivate* d;
	};
}

#define YSSLSM Visindigo::Editor::LangServerManager::getInstance()