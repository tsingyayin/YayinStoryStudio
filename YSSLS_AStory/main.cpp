#include "Macro.h"
#include <Editor/LangServer.h>
#include "AStory.h"

#ifdef YSSLS_AStory_EXPORT
extern "C" YSSLS_AStory YSSCore::Editor::LangServer* YSSLangServerDllMain()
{
	return new AStoryLanguageServer();
}
#endif

#ifdef YSSLS_AStory_DEVELOPMENT
int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	return 0;
}
#endif

