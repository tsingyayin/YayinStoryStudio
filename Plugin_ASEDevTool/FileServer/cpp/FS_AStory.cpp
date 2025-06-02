#include "../FS_AStory.h"

AStoryFileServer::AStoryFileServer(YSSCore::Editor::EditorPlugin* plugin):FileServer(plugin) {
	setEditorType(YSSCore::Editor::FileServer::CodeEditor);
	setSupportedFileExts({ "astory" });
}

AStoryFileServer::~AStoryFileServer() {
	FileServer::~FileServer();
}