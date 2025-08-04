#pragma once
#include <Editor/FileServer.h>
#include <Editor/EditorPlugin.h>

class AStoryFileServer : public Visindigo::Editor::FileServer {
public:
	AStoryFileServer(Visindigo::Editor::EditorPlugin* plugin);
	~AStoryFileServer();
};