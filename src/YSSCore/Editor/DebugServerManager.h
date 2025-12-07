#ifndef YSSCore_Editor_DebugServerManager_h
#define YSSCore_Editor_DebugServerManager_h
#include "../Macro.h"
#include "DebugServer.h"
namespace YSSCore::Editor {
	class DebugServerManagerPrivate;

	class YSSCoreAPI DebugServerManager {
	private:
		DebugServerManager();
	public:
		~DebugServerManager();
		static DebugServerManager* getInstance();
		DebugServer* getDebugServer(const QString& serverID);
		void addDebugServer(DebugServer* server);
	private:
		DebugServerManagerPrivate* d;
	};
}
#define YSSDSM YSSCore::Editor::DebugServerManager::getInstance()
#endif // YSSCore_Editor_DebugServerManager_h
