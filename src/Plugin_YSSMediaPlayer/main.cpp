#include "MediaPlayerCompileMacro.h"
#include "Plugin_YSSMediaPlayer.h"
extern "C" MediaPlayerAPI Visindigo::General::Plugin* VisindigoPluginMain() {
	return new YSS::MediaPlayer::Main();
}
#ifdef Q_OS_ANDROID
extern "C" MediaPlayerAPI Visindigo::General::Plugin* VisindigoPluginMain_MediaPlayer() {
	return VisindigoPluginMain();
}
#endif
