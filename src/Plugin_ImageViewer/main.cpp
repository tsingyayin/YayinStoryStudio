#include "Plugin_ImageViewer.h"
#include "ImageViewerCompileMacro.h"
extern "C" ImageViewerAPI Visindigo::General::Plugin* VisindigoPluginMain() {
	return new YSS::ImageViewer::Main();
}