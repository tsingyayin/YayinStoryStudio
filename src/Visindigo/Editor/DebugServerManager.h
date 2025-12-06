#ifndef Visindigo_Editor_DebugServerManager_h
#define Visindigo_Editor_DebugServerManager_h
#include "DebugServer.h"

namespace Visindigo::Editor {
	class DebugServerManagerPrivate;

	class VisindigoAPI DebugServerManager {
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
#define YSSDSM Visindigo::Editor::DebugServerManager::getInstance()
#endif
