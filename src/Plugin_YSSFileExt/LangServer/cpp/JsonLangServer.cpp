#include "../JsonLangServer.h"
#include <QtCore/qregularexpression.h>
namespace YSSFileExt {
	JsonLangServer::JsonLangServer(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::LangServer("YSS File Extension Json Language Server",
			"YSSFileExt_Json", plugin, "Json", {"json", "yssp"})
	{
		
	}

	YSSCore::Editor::SyntaxHighlighter* JsonLangServer::createHighlighter(YSSCore::Editor::TextEdit* parent) {
		return new JsonLangHighlighter(parent);
	}

	JsonLangHighlighter::JsonLangHighlighter(YSSCore::Editor::TextEdit* parent) :
		YSSCore::Editor::SyntaxHighlighter(parent)
	{
	}

	void JsonLangHighlighter::onBlockChanged(const QString& text, int blockNumber) {
		// Simple JSON syntax highlighting
		QRegularExpression keyRegex("\".+\": ");

		auto keyMatches = keyRegex.globalMatch(text);
		while (keyMatches.hasNext()) {
			auto match = keyMatches.next();
			setFormat(match.capturedStart(), match.capturedLength(), QColor("#D69D85")); // red for keys
		}
	}
}