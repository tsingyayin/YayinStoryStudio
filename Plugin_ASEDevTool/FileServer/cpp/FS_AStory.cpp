#include "../FS_AStory.h"

AStoryFileServer::AStoryFileServer(YSSCore::Editor::EditorPlugin* plugin) :
	FileServer("ASE AStory File Server", "ASEAStoryFS", plugin)
{
	setEditorType(YSSCore::Editor::FileServer::CodeEditor);
	setSupportedFileExts({ "astory" });
}

AStoryFileServer::~AStoryFileServer() {
}