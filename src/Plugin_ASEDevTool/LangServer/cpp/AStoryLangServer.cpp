#include "../AStoryLangServer.h"
#include "../AStoryHighlighter.h"
#include "../AStoryTabCompleterProvider.h"
#include <QtGui/qtextdocument.h>

AStoryLanguageServer::AStoryLanguageServer(Visindigo::Editor::EditorPlugin* plugin) :
	Visindigo::Editor::LangServer("ASE AStory File Language Server", "ASEAStoryLS", plugin, "AStory", { "astory" })
{
}

QSyntaxHighlighter* AStoryLanguageServer::createHighlighter(QTextDocument* doc) {
	return new AStorySyntaxHighlighter(doc);
}

Visindigo::Editor::TabCompleterProvider* AStoryLanguageServer::createTabCompleter(QTextDocument* doc) {
	return new AStoryTabCompleterProvider(doc);
}