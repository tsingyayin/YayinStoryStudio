#ifndef ASERStudio_YSS_DS_AStoryXDebugger_h
#define ASERStudio_YSS_DS_AStoryXDebugger_h
#include "ASERStudioCompileMacro.h"
#include <Editor/DebugServer.h>
#define ASERStudio_AStoryXDebuggerID "cn.yxgeneral.aserstudio.astoryxdebugger"
namespace ASERStudio::YSS {
	class DS_AStoryXDebuggerPrivate;
	class DS_AStoryXDebugger :public YSSCore::Editor::DebugServer {
	public:
		DS_AStoryXDebugger(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~DS_AStoryXDebugger();
		virtual void onRun(bool debug) override;
		virtual void onStop() override;
	private:
		DS_AStoryXDebuggerPrivate* d;
	};
}
#endif // ASERStudio_YSS_DS_AStoryXDebugger_h
