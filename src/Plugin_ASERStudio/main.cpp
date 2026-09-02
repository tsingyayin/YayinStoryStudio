#include "ASERStudioCompileMacro.h"
#include "Plugin_ASERStudio.h"
extern "C" ASERAPI Visindigo::General::Plugin* VisindigoPluginMain() {
	return new ASERStudio::Main();
}