#include "YSS/LS_AStoryXSyntaxHighlighter.h"
#include "AStorySyntax/AStoryXDocument.h"
#include "YSS/LangServer_AStoryX.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include <General/Log.h>
namespace ASERStudio::YSS {
	class LS_AStoryXSyntaxHighlighterPrivate {
		friend class LS_AStoryXSyntaxHighlighter;
	protected:
		AStorySyntax::AStoryXDocument* Document = nullptr;
	};
	LS_AStoryXSyntaxHighlighter::LS_AStoryXSyntaxHighlighter(YSSCore::Editor::TextEdit* parent) : SyntaxHighlighter(parent) {
		d = new LS_AStoryXSyntaxHighlighterPrivate();
		d->Document = AStoryXLanguageServer::getAStoryXDocument(parent->getDocument());
	}
	LS_AStoryXSyntaxHighlighter::~LS_AStoryXSyntaxHighlighter() {
		AStoryXLanguageServer::deleteAStoryXDocument(document());
		delete d;
	}
	void LS_AStoryXSyntaxHighlighter::onBlockChanged(const QString& text, int blockNumber) {
		if (!d->Document) return;
		auto parseData = d->Document->getParseData(blockNumber);
		vgDebug << text<<blockNumber << parseData;
	}
}