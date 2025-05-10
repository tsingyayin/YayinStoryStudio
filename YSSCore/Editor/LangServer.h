#pragma once
#include <QList>
#include "../Macro.h"

class QString;
class QSyntaxHighlighter;

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
		LangServerPrivate* d;
	};
}
