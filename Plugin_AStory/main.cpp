#include "Macro.h"
#include "Plugin_AStory.h"

#ifdef AStory_EXPORT
extern "C" AStoryAPI YSSCore::Editor::EditorPlugin* YSSPluginDllMain(void)
{
	return new Plugin_AStory();
}
#endif

#ifdef AStory_DEVELOPMENT
int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	return 0;
}
#endif

