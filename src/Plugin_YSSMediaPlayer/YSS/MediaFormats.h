#ifndef Plugin_YSSMediaPlayer_YSS_MediaFormats_h
#define Plugin_YSSMediaPlayer_YSS_MediaFormats_h
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

namespace YSS::MediaPlayer {
	/*
		与 Qt Multimedia（Windows 下默认使用 FFmpeg 后端）能够解码的容器格式对应的文件扩展名。
		这里只登记扩展名，实际能否播放仍由 Qt 在运行时的解码器能力决定。
	*/
	namespace MediaFormats {
		inline const QStringList VideoExtensions = {
			// 注意：不登记 "ts"（MPEG-TS），因为 YSSFileExt 的文本编辑器已将其用于 TypeScript。
			// 需要 MPEG-TS 时请使用 "m2ts" / "mts"。
			"mp4", "m4v", "mov", "avi", "mkv", "webm", "wmv", "asf", "flv", "f4v",
			"mpg", "mpeg", "mpe", "m1v", "m2v", "m2ts", "mts", "3gp", "3g2",
			"ogv", "ogm", "vob", "mxf", "dv", "y4m", "rm", "rmvb"
		};
		inline const QStringList AudioExtensions = {
			"mp3", "mp2", "mpga", "m4a", "m4b", "aac", "wav", "wave", "flac", "ogg",
			"oga", "opus", "wma", "mka", "aiff", "aif", "aifc", "alac", "amr", "awb",
			"ac3", "eac3", "dts", "ape", "wv", "tta", "caf", "au", "snd", "voc", "ra"
		};

		inline QStringList allExtensions() {
			return VideoExtensions + AudioExtensions;
		}

		inline bool isVideoExtension(const QString& ext) {
			return VideoExtensions.contains(ext.toLower());
		}
	}
}
#endif // Plugin_YSSMediaPlayer_YSS_MediaFormats_h
