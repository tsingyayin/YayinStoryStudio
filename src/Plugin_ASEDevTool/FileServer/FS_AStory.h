#pragma once
#include <Editor/FileServer.h>
#include <Editor/EditorPlugin.h>

class AStoryFileServer : public YSSCore::Editor::FileServer {
public:
	AStoryFileServer(YSSCore::Editor::EditorPlugin* plugin);
	~AStoryFileServer();
};