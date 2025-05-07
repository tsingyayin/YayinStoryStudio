#pragma once
#include <QString>
#include <QList>
#include "../Macro.h"
#include <QSyntaxHighlighter>

namespace YSSCore::Editor {
	class LangServerPrivate;
	class YSSCoreAPI LangServer {
		friend class LangServerPrivate;
		friend class LangServerManager;
	public:
		LangServer(QString id, QStringList ext);
		~LangServer();
		virtual QSyntaxHighlighter* createHighlighter() = 0;
		QString getLangID();
		QStringList getLangExts();
	protected:
		LangServerPrivate* p;
	};
}
