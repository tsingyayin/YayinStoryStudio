#ifndef ASERStudio_YSS_LS_AStoryXTabCompleter_h
#define ASERStudio_YSS_LS_AStoryXTabCompleter_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include <Editor/TabCompleterProvider.h>
namespace ASERStudio::YSS {
	class LS_AStoryXTabCompleterPrivate;
	class ASERAPI LS_AStoryXTabCompleter :public YSSCore::Editor::TabCompleterProvider {
		Q_OBJECT;
	public:
		LS_AStoryXTabCompleter(YSSCore::Editor::TextEdit* textEdit);
		virtual ~LS_AStoryXTabCompleter();
		void onSpecificParameters(QList<YSSCore::Editor::TabCompleterItem>* items, ASERStudio::AStorySyntax::AStoryXControllerParseData* data, qint32 column);
		virtual QList<YSSCore::Editor::TabCompleterItem> onTabComplete(qint32 line, qint32 column, const QString& content) override;
	private:
		LS_AStoryXTabCompleterPrivate* d;
	};
}
#endif // ASERStudio_YSS_LS_AStoryXTabCompleter_h