#pragma once
#include <Editor/DebugServer.h>

class DS_ASE :public YSSCore::Editor::DebugServer {
public:
	DS_ASE(YSSCore::Editor::EditorPlugin* plugin);
};