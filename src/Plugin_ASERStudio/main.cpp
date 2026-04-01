#include "Plugin_ASERStudio.h"
#include "ASERStudioCompileMacro.h"
extern "C" ASERAPI Visindigo::General::Plugin* VisindigoPluginMain() {
	return new ASERStudio::Main();
}