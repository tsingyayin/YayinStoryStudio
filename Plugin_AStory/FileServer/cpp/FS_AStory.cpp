#include "../FS_AStory.h"

AStoryFileServer::AStoryFileServer() {
	setEditorType(YSSCore::Editor::FileServer::CodeEditor);
	setSupportedFileExts({ "astory" });
}