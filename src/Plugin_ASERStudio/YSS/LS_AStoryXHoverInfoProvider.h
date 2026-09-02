#ifndef Plugin_ASERStudio_YSS_LS_AStoryXHoverInfoProvider_h
#define Plugin_ASERStudio_YSS_LS_AStoryXHoverInfoProvider_h
#include "ASERStudioCompileMacro.h"
#include <Editor/HoverInfoProvider.h>

namespace ASERStudio::YSS {
	class LS_AStoryXHoverInfoProviderPrivate;
	class LS_AStoryXHoverInfoProvider :public YSSCore::Editor::HoverInfoProvider {
	public:
		LS_AStoryXHoverInfoProvider(YSSCore::Editor::TextEdit* textEdit);
		~LS_AStoryXHoverInfoProvider();
		virtual void onMouseHover(qint32 lineNumber, qint32 column, const QString& content) override;
	private:
		LS_AStoryXHoverInfoProviderPrivate* d;
	};
}
#endif // Plugin_ASERStudio_YSS_LS_AStoryXHoverInfoProvider_h
