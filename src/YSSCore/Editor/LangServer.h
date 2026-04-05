#pragma once
#include "../Macro.h"
#include <General/PluginModule.h>
// Forward declarations
class QString;
class QTextDocument;
namespace YSSCore::Editor {
	class EditorPlugin;
	class TabCompleterProvider;
	class HoverInfoProvider;
	class LangServerPrivate;
	class SyntaxHighlighter;
	class TextEdit;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI LangServer :public Visindigo::General::PluginModule {
		friend class LangServerPrivate;
		friend class LangServerManager;
	public:
		LangServer(const QString& name, const QString& id, EditorPlugin* plugin, const QString& lang_id, QStringList ext);
		virtual ~LangServer();
		virtual SyntaxHighlighter* createHighlighter(TextEdit* doc) = 0;
		virtual TabCompleterProvider* createTabCompleter(TextEdit* doc);
		virtual HoverInfoProvider* createHoverInfoProvider(TextEdit* doc);
		QString getLangID();
		QStringList getLangExts();
	protected:
		LangServerPrivate* d;
	};
}
