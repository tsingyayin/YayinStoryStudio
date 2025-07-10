#pragma once
#include <QList>
#include "../Macro.h"

class QString;
class QTextDocument;
class QSyntaxHighlighter;
namespace YSSCore::Editor {
	class EditorPlugin;
	class TabCompleterProvider;
	class LangServerPrivate;
	class YSSCoreAPI LangServer {
		friend class LangServerPrivate;
		friend class LangServerManager;
	public:
		LangServer(EditorPlugin* plugin, QString id, QStringList ext);
		virtual ~LangServer();
		virtual QSyntaxHighlighter* createHighlighter(QTextDocument* doc) = 0;
		virtual TabCompleterProvider* createTabCompleter(QTextDocument* doc) = 0;
		QString getLangID();
		QStringList getLangExts();
		EditorPlugin* getPlugin();
	protected:
		LangServerPrivate* d;
	};
}
