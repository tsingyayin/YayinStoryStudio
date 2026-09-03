#include "ASERStudioCompileMacro.h"
#include "Plugin_ASERStudio.h"
extern "C" ASERAPI Visindigo::General::Plugin* VisindigoPluginMain() {
	return new ASERStudio::Main();
}
#ifdef Q_OS_ANDROID
extern "C" ASERAPI Visindigo::General::Plugin* VisindigoPluginMain_ASERStudio() {
	return VisindigoPluginMain();
}
#endif