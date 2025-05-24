#include "Macro.h"
#include "Plugin_ASEDevTool.h"

#ifdef AStory_EXPORT
extern "C" AStoryAPI YSSCore::Editor::EditorPlugin* YSSPluginDllMain(void)
{
	return new Plugin_ASEDevTool();
}
#endif

#ifdef AStory_DEVELOPMENT
int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	return 0;
}
#endif