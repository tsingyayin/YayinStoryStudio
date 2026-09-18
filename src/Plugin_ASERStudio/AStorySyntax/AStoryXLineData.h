#ifndef Plugin_ASERStudio_AStorySyntax_AStoryXLineData_h
#define Plugin_ASERStudio_AStorySyntax_AStoryXLineData_h
#include "ASERStudioCompileMacro.h"
#include <Editor/CustomDocumentData.h>
#include "AStorySyntax/AStoryXControllerParseData.h"

namespace ASERStudio::AStorySyntax {
	class ASERAPI AStoryXLineData :public YSSCore::Editor::ICustomDocumentData {
	public:
		AStoryXControllerParseData ParseData;
	};
}
#endif // Plugin_ASERStudio_AStorySyntax_AStoryXLineData_h
