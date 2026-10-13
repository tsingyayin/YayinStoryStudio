#ifndef Plugin_YSSMediaPlayer_MediaPlayerCompileMacro_h
#define Plugin_YSSMediaPlayer_MediaPlayerCompileMacro_h
#ifdef _MSC_VER
#define MediaPlayerAPI __declspec(dllimport)
#ifdef YSSMEDIAPLAYER_DEVELOPMENT
#undef MediaPlayerAPI
#define MediaPlayerAPI
#endif
#ifdef YSSMEDIAPLAYER_EXPORT
#undef MediaPlayerAPI
#define MediaPlayerAPI __declspec(dllexport)
#endif
#else
// 非 MSVC（Android/Clang/GCC/…）：默认可见性即可，无需 __declspec。
#define MediaPlayerAPI
#endif
#endif // Plugin_YSSMediaPlayer_MediaPlayerCompileMacro_h
