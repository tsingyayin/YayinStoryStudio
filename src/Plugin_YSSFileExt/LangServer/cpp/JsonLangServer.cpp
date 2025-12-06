#include "../JsonLangServer.h"
#include <QtCore/qregularexpression.h>
namespace YSSFileExt {
	JsonLangServer::JsonLangServer(Visindigo::Editor::EditorPlugin* plugin) :
		Visindigo::Editor::LangServer("YSS File Extension Json Language Server",
			"YSSFileExt_Json", plugin, "Json", {"json", "yssp"})
	{
		
	}

	QSyntaxHighlighter* JsonLangServer::createHighlighter(QTextDocument* doc) {
		return new JsonLangHighlighter(doc);
	}

	Visindigo::Editor::TabCompleterProvider* JsonLangServer::createTabCompleter(QTextDocument* doc) {
		return nullptr; // TODO
	}

	JsonLangHighlighter::JsonLangHighlighter(QTextDocument* parent) :
		QSyntaxHighlighter(parent)
	{
	}

	void JsonLangHighlighter::highlightBlock(const QString& text) {
		// Simple JSON syntax highlighting
		QRegularExpression keyRegex("\".+\": ");

		auto keyMatches = keyRegex.globalMatch(text);
		while (keyMatches.hasNext()) {
			auto match = keyMatches.next();
			setFormat(match.capturedStart(), match.capturedLength(), QColor("#D69D85")); // red for keys
		}
	}
}