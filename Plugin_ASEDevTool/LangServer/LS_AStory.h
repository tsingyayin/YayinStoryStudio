#pragma once
#include <Editor/LangServer.h>

class AStoryLanguageServer :public YSSCore::Editor::LangServer {
public:
	AStoryLanguageServer();
	virtual QSyntaxHighlighter* createHighlighter() override;
};