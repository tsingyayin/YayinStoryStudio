#include "../AStoryLangServer.h"
#include "../AStoryHighlighter.h"
#include "../AStoryTabCompleterProvider.h"
#include <QtGui/qtextdocument.h>

AStoryLanguageServer::AStoryLanguageServer(YSSCore::Editor::EditorPlugin* plugin) :
	YSSCore::Editor::LangServer("ASE AStory File Language Server", "ASEAStoryLS", plugin, "AStory", { "astory" })
{
}

QSyntaxHighlighter* AStoryLanguageServer::createHighlighter(QTextDocument* doc) {
	return new AStorySyntaxHighlighter(doc);
}

YSSCore::Editor::TabCompleterProvider* AStoryLanguageServer::createTabCompleter(QTextDocument* doc) {
	return new AStoryTabCompleterProvider(doc);
}