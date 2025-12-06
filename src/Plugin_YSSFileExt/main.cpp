#include "Macro.h"
#include <General/Plugin.h>
#include "Plugin_YSSFileExt.h"
#ifdef YSSFileExt_EXPORT
extern "C" YSSFileExtAPI Visindigo::General::Plugin* VisindigoPluginMain(void)
{
	return new Plugin_YSSFileExt();
}
#endif

#ifdef YSSFileExt_DEVELOPMENT
int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	return 0;
}
#endif