#ifndef ASERStudio_YSS_LS_AStoryXSyntaxHighlighter_h
#define ASERStudio_YSS_LS_AStoryXSyntaxHighlighter_h
#include "ASERStudioCompileMacro.h"
#include <Editor/SyntaxHighlighter.h>
#include <Editor/TextEdit.h>
namespace ASERStudio::YSS {
	class LS_AStoryXSyntaxHighlighterPrivate;
	class LS_AStoryXSyntaxHighlighter :public YSSCore::Editor::SyntaxHighlighter {
		Q_OBJECT;
	public:
		LS_AStoryXSyntaxHighlighter(YSSCore::Editor::TextEdit* parent = nullptr);
		virtual ~LS_AStoryXSyntaxHighlighter();
		virtual void onBlockChanged(const QString& text, int blockNumber) override;
	private:
		LS_AStoryXSyntaxHighlighterPrivate* d;
	};
}
#endif //ASERStudio_YSS_LS_AStoryXSyntaxHighlighter_h