#pragma once
#include <QList>
#include "../Macro.h"

class QString;
class QSyntaxHighlighter;

namespace YSSCore::Editor {
	class EditorPlugin;
	class LangServerPrivate;
	class YSSCoreAPI LangServer {
		friend class LangServerPrivate;
		friend class LangServerManager;
	public:
		LangServer(EditorPlugin* plugin, QString id, QStringList ext);
		virtual ~LangServer();
		virtual QSyntaxHighlighter* createHighlighter() = 0;
		QString getLangID();
		QStringList getLangExts();
		EditorPlugin* getPlugin();
	protected:
		LangServerPrivate* d;
	};
}
