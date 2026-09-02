#include "ImageViewerCompileMacro.h"
#include "Plugin_ImageViewer.h"
extern "C" ImageViewerAPI Visindigo::General::Plugin* VisindigoPluginMain() {
	return new YSS::ImageViewer::Main();
}