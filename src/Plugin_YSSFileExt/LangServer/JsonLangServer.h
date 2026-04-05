#pragma once
#include <Editor/LangServer.h>
#include <Editor/EditorPlugin.h>
#include <Editor/SyntaxHighlighter.h>

namespace YSSFileExt {
	class JsonLangServer :public YSSCore::Editor::LangServer {
	public:
		JsonLangServer(YSSCore::Editor::EditorPlugin* plugin);
		virtual YSSCore::Editor::SyntaxHighlighter* createHighlighter(YSSCore::Editor::TextEdit* doc) override;
	};

	class JsonLangHighlighter :public YSSCore::Editor::SyntaxHighlighter {
		Q_OBJECT;
	public:
		JsonLangHighlighter(YSSCore::Editor::TextEdit* parent = nullptr);
		virtual void onBlockChanged(const QString& text, int blockNumber) override;
	};
}