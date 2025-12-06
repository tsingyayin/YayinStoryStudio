#pragma once
#include <Editor/LangServer.h>
#include <Editor/EditorPlugin.h>
#include <QtGui/qsyntaxhighlighter.h>

namespace YSSFileExt {
	class JsonLangServer :public Visindigo::Editor::LangServer {
	public:
		JsonLangServer(Visindigo::Editor::EditorPlugin* plugin);
		virtual QSyntaxHighlighter* createHighlighter(QTextDocument* doc) override;
		virtual Visindigo::Editor::TabCompleterProvider* createTabCompleter(QTextDocument* doc) override;
		virtual Visindigo::Editor::HoverInfoProvider* createHoverInfoProvider(QTextDocument* doc) override {
			return nullptr; //TODO
		}
	};

	class JsonLangHighlighter :public QSyntaxHighlighter {
		Q_OBJECT;
	public:
		JsonLangHighlighter(QTextDocument* parent = nullptr);
		virtual void highlightBlock(const QString& text) override;
	};
}