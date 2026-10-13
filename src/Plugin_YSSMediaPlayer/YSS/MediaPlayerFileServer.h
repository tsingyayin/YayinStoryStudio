#ifndef Plugin_YSSMediaPlayer_YSS_MediaPlayerFileServer_h
#define Plugin_YSSMediaPlayer_YSS_MediaPlayerFileServer_h
#include <Editor/FileServer.h>
#include "MediaPlayerCompileMacro.h"

namespace YSS::MediaPlayer {
	class MediaPlayerAPI FS_MediaPlayer :public YSSCore::Editor::FileServer {
	public:
		FS_MediaPlayer(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~FS_MediaPlayer();
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};
}
#endif // Plugin_YSSMediaPlayer_YSS_MediaPlayerFileServer_h
