#include "LangServer/AStoryLangServer.h"
#include "LangServer/AStoryHighlighter.h"
#include "LangServer/AStoryTabCompleterProvider.h"
#include "LangServer/ASEHoverInfoProvider.h"
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

YSSCore::Editor::HoverInfoProvider* AStoryLanguageServer::createHoverInfoProvider(QTextDocument* doc) {
	return new AStoryHoverInfoProvider(doc);
}