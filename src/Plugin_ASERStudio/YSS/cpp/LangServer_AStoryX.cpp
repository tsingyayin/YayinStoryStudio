#include "YSS/LangServer_AStoryX.h"
#include "YSS/LS_AStoryXSyntaxHighlighter.h"
#include "YSS/LS_AStoryXTabCompleter.h"
namespace ASERStudio::YSS {
	class AStoryXLanguageServerPrivate {
		friend class AStoryXLanguageServer;
	protected:
		static QMap<QString, ASERStudio::AStorySyntax::AStoryXDocument*> DocumentMap;
	};
	QMap<QString, ASERStudio::AStorySyntax::AStoryXDocument*> AStoryXLanguageServerPrivate::DocumentMap;

	AStoryXLanguageServer::AStoryXLanguageServer(YSSCore::Editor::EditorPlugin* plugin) : 
		LangServer("ASE-Remake AStoryX File Language Server", "ASEAStoryXLS", plugin, "AStoryX", { "astoryx" }) {
	}
	YSSCore::Editor::SyntaxHighlighter* AStoryXLanguageServer::createHighlighter(YSSCore::Editor::TextEdit* textEdit) {
		
		return new LS_AStoryXSyntaxHighlighter(textEdit);
	}
	YSSCore::Editor::TabCompleterProvider* AStoryXLanguageServer::createTabCompleter(YSSCore::Editor::TextEdit* textEdit) {
		return new LS_AStoryXTabCompleter(textEdit);
	}
	YSSCore::Editor::HoverInfoProvider* AStoryXLanguageServer::createHoverInfoProvider(YSSCore::Editor::TextEdit* textEdit) {
		return nullptr;
	}
	void AStoryXLanguageServer::setAStoryXDocument(const QString& who, ASERStudio::AStorySyntax::AStoryXDocument* doc) {
		AStoryXLanguageServerPrivate::DocumentMap.insert(who, doc);
	}
	void AStoryXLanguageServer::deleteAStoryXDocument(const QString& who) {
		ASERStudio::AStorySyntax::AStoryXDocument* doc = AStoryXLanguageServerPrivate::DocumentMap.value(who, nullptr);
		if (doc) {
			delete doc;
		}
		AStoryXLanguageServerPrivate::DocumentMap.remove(who);
	}
	ASERStudio::AStorySyntax::AStoryXDocument* AStoryXLanguageServer::getAStoryXDocument(const QString& who) {
		return AStoryXLanguageServerPrivate::DocumentMap.value(who, nullptr);
	}
}