#ifndef ASERStudio_YSS_LangServer_AStoryX_h
#define ASERStudio_YSS_LangServer_AStoryX_h
#include <Editor/LangServer.h>
#include <Editor/EditorPlugin.h>
#include "AStorySyntax/AStoryXDocument.h"
namespace ASERStudio::YSS {
	class AStoryXLanguageServer :public YSSCore::Editor::LangServer {
	public:
		AStoryXLanguageServer(YSSCore::Editor::EditorPlugin* plugin);
		virtual YSSCore::Editor::SyntaxHighlighter* createHighlighter(YSSCore::Editor::TextEdit* textEdit) override;
		virtual YSSCore::Editor::TabCompleterProvider* createTabCompleter(YSSCore::Editor::TextEdit* doc) override;
		virtual YSSCore::Editor::HoverInfoProvider* createHoverInfoProvider(YSSCore::Editor::TextEdit* doc) override;
		static void setAStoryXDocument(const QString& filePath, ASERStudio::AStorySyntax::AStoryXDocument* doc);
		static void deleteAStoryXDocument(const QString& filePath);
		static ASERStudio::AStorySyntax::AStoryXDocument* getAStoryXDocument(const QString& filePath);
	};
}

#endif // ASERStudio_YSS_LangServer_AStoryX_h
