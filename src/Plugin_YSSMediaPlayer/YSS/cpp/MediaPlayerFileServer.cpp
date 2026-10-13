#include "YSS/MediaPlayerFileServer.h"
#include "YSS/MediaFormats.h"
#include "YSS/MediaPlayerWidget.h"

namespace YSS::MediaPlayer {
	FS_MediaPlayer::FS_MediaPlayer(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("YSSMediaPlayer", "cn.yxgeneral.yayinstorystudio.plugin.yssmediaplayer.fs", plugin) {
		setEditorType(EditorType::BuiltInEditor);
		setSupportedFileExts(MediaFormats::allExtensions());
	}

	FS_MediaPlayer::~FS_MediaPlayer() {}

	YSSCore::Editor::FileEditWidget* FS_MediaPlayer::onCreateFileEditWidget() {
		return new MediaPlayerWidget();
	}
}
