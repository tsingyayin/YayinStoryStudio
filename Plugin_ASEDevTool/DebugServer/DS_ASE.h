#pragma once
#include <Editor/DebugServer.h>

class DS_ASE :public Visindigo::Editor::DebugServer {
public:
	DS_ASE(Visindigo::Editor::EditorPlugin* plugin);
};