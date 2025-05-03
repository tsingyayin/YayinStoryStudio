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
		void scan();
		bool addLangServer(LangServer* server);
		LangServer* routeID(const QString& id);
		LangServer* routeExt(const QString& fileExt);
		void removeServer(LangServer* server);
	private:
		LangServerManagerPrivate* p;
	};
}