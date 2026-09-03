#include "ImageViewerCompileMacro.h"
#include "Plugin_ImageViewer.h"
extern "C" ImageViewerAPI Visindigo::General::Plugin* VisindigoPluginMain() {
	return new YSS::ImageViewer::Main();
}
#ifdef Q_OS_ANDROID
extern "C" ImageViewerAPI Visindigo::General::Plugin* VisindigoPluginMain_ImageViewer() {
	return VisindigoPluginMain();
}
#endif