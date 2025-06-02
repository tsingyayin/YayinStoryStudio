#pragma once
#include <Editor/LangServer.h>
#include <Editor/EditorPlugin.h>

class AStoryLanguageServer :public YSSCore::Editor::LangServer {
public:
	AStoryLanguageServer(YSSCore::Editor::EditorPlugin* plugin);
	virtual QSyntaxHighlighter* createHighlighter() override;
};