#include "../LS_AStory.h"
#include "../SH_AStory.h"

AStoryLanguageServer::AStoryLanguageServer(YSSCore::Editor::EditorPlugin* plugin) :
	YSSCore::Editor::LangServer(plugin, "AStory", { "astory" })
{
}

QSyntaxHighlighter* AStoryLanguageServer::createHighlighter() {
	return new AStorySyntaxHighlighter();
}