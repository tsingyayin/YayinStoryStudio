#pragma once
#include "../Macro.h"

class QString;
namespace Visindigo::Editor {
	class LangServer;
	class LangServerManagerPrivate;
	class VisindigoAPI LangServerManager {
	private:
		LangServerManager();
	public:
		~LangServerManager();
		static LangServerManager* getInstance();
		bool addLangServer(LangServer* server);
		LangServer* routeID(const QString& id);
		LangServer* routeExt(const QString& fileExt);
		void removeServer(LangServer* server);
	private:
		LangServerManagerPrivate* d;
	};
}

#define YSSLSM Visindigo::Editor::LangServerManager::getInstance()