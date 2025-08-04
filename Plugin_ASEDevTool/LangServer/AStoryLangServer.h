#pragma once
#include <Editor/LangServer.h>
#include <Editor/EditorPlugin.h>

class AStoryLanguageServer :public Visindigo::Editor::LangServer {
public:
	AStoryLanguageServer(Visindigo::Editor::EditorPlugin* plugin);
	virtual QSyntaxHighlighter* createHighlighter(QTextDocument* doc) override;
	virtual Visindigo::Editor::TabCompleterProvider* createTabCompleter(QTextDocument* doc) override;
	virtual Visindigo::Editor::HoverInfoProvider* createHoverInfoProvider(QTextDocument* doc) override {
		return nullptr; //TODO
	}
};