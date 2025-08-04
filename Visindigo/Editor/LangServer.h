#pragma once
#include <QtCore/qlist.h>
#include "EditorPluginModule.h"
// Forward declarations
class QString;
class QTextDocument;
class QSyntaxHighlighter;
namespace Visindigo::Editor {
	class EditorPlugin;
	class TabCompleterProvider;
	class HoverInfoProvider;
	class LangServerPrivate;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI LangServer :public EditorPluginModule {
		friend class LangServerPrivate;
		friend class LangServerManager;
	public:
		LangServer(const QString& name, const QString& id, EditorPlugin* plugin, const QString& lang_id, QStringList ext);
		virtual ~LangServer();
		virtual QSyntaxHighlighter* createHighlighter(QTextDocument* doc) = 0;
		virtual TabCompleterProvider* createTabCompleter(QTextDocument* doc) = 0;
		virtual HoverInfoProvider* createHoverInfoProvider(QTextDocument* doc) = 0;
		QString getLangID();
		QStringList getLangExts();
	protected:
		LangServerPrivate* d;
	};
}
