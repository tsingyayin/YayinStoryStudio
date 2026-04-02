#include "YSS/LangServer_AStoryX.h"
#include "YSS/LS_AStoryXSyntaxHighlighter.h"

namespace ASERStudio::YSS {
	class AStoryXLanguageServerPrivate {
		friend class AStoryXLanguageServer;
	protected:
		static QMap<QTextDocument*, ASERStudio::AStorySyntax::AStoryXDocument*> DocumentMap;
	};
	QMap<QTextDocument*, ASERStudio::AStorySyntax::AStoryXDocument*> AStoryXLanguageServerPrivate::DocumentMap;

	AStoryXLanguageServer::AStoryXLanguageServer(YSSCore::Editor::EditorPlugin* plugin) : 
		LangServer("ASE-Remake AStoryX File Language Server", "ASEAStoryXLS", plugin, "AStoryX", { "astoryx" }) {
	}
	YSSCore::Editor::SyntaxHighlighter* AStoryXLanguageServer::createHighlighter(YSSCore::Editor::TextEdit* textEdit) {
		// Why put the document creation here?
		// AStoryXDocument also have a highlighter inside itself to spy on the text document change
		// and generate syntax info. So if we want to get correct syntax info in the "truely" highlighting process,
		// the AStoryXDocument must be created firstly, so it will get high priority in signal-slot connection
		// and get the text change signal before the highlighter, then update the syntax info for the highlighter to use.
		ASERStudio::AStorySyntax::AStoryXDocument* doc = new ASERStudio::AStorySyntax::AStoryXDocument();
		doc->setEnableDiagnostic(true);
		doc->setTextDocument(textEdit->getDocument());
		setAStoryXDocument(textEdit->getDocument(), doc);
		return new LS_AStoryXSyntaxHighlighter(textEdit);
	}
	YSSCore::Editor::TabCompleterProvider* AStoryXLanguageServer::createTabCompleter(QTextDocument* doc) {
		return nullptr;
	}
	YSSCore::Editor::HoverInfoProvider* AStoryXLanguageServer::createHoverInfoProvider(QTextDocument* doc) {
		return nullptr;
	}
	void AStoryXLanguageServer::setAStoryXDocument(QTextDocument* who, ASERStudio::AStorySyntax::AStoryXDocument* doc) {
		AStoryXLanguageServerPrivate::DocumentMap.insert(who, doc);
	}
	void AStoryXLanguageServer::deleteAStoryXDocument(QTextDocument* who) {
		ASERStudio::AStorySyntax::AStoryXDocument* doc = AStoryXLanguageServerPrivate::DocumentMap.value(who, nullptr);
		if (doc) {
			delete doc;
		}
		AStoryXLanguageServerPrivate::DocumentMap.remove(who);
	}
	ASERStudio::AStorySyntax::AStoryXDocument* AStoryXLanguageServer::getAStoryXDocument(QTextDocument* who) {
		return AStoryXLanguageServerPrivate::DocumentMap.value(who, nullptr);
	}
}