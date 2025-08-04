#include "../FS_AStory.h"

AStoryFileServer::AStoryFileServer(Visindigo::Editor::EditorPlugin* plugin) :
	FileServer("ASE AStory File Server", "ASEAStoryFS", plugin)
{
	setEditorType(Visindigo::Editor::FileServer::CodeEditor);
	setSupportedFileExts({ "astory" });
}

AStoryFileServer::~AStoryFileServer() {
}