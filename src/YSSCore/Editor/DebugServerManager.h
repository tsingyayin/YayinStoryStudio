#ifndef YSSCore_Editor_DebugServerManager_h
#define YSSCore_Editor_DebugServerManager_h
#include "YSSCoreCompileMacro.h"
#include "DebugServer.h"
#include <QtCore/qobject.h>
namespace YSSCore::Editor {
	class DebugServerManagerPrivate;

	class YSSCoreAPI DebugServerManager :public QObject{
		Q_OBJECT;
	private:
		DebugServerManager();
	signals:
		void projectDebugServerChanged(const QString& serverID);
	public:
		~DebugServerManager();
		static DebugServerManager* getInstance();
		QStringList getDebugServerIDs();
		DebugServer* getDebugServer(const QString& serverID);
		void addDebugServer(DebugServer* server);
	private:
		DebugServerManagerPrivate* d;
	};
}
#define YSSDSM YSSCore::Editor::DebugServerManager::getInstance()
#endif // YSSCore_Editor_DebugServerManager_h
