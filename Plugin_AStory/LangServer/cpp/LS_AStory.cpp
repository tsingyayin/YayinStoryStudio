#include "../LS_AStory.h"
#include "../SH_AStory.h"

AStoryLanguageServer::AStoryLanguageServer() :
	YSSCore::Editor::LangServer("AStory", { "astory" })
{

}

QSyntaxHighlighter* AStoryLanguageServer::createHighlighter() {
	return new AStorySyntaxHighlighter();
}