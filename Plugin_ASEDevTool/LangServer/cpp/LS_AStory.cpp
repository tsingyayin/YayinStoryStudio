#include "../LS_AStory.h"
#include "../SH_AStory.h"
#include <QtGui/qtextdocument.h>
AStoryLanguageServer::AStoryLanguageServer(YSSCore::Editor::EditorPlugin* plugin) :
	YSSCore::Editor::LangServer(plugin, "AStory", { "astory" })
{
}

QSyntaxHighlighter* AStoryLanguageServer::createHighlighter(QTextDocument* doc) {
	return new AStorySyntaxHighlighter(doc);
}

YSSCore::Editor::TabCompleterProvider* AStoryLanguageServer::createTabCompleter(QTextDocument* doc) {
	return nullptr; //TODO
}