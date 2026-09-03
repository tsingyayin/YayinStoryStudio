#ifndef Plugin_ImageViewer_ImageViewerCompileMacro_h
#define Plugin_ImageViewer_ImageViewerCompileMacro_h
#ifdef _MSC_VER
#define ImageViewerAPI __declspec(dllimport)
#ifdef IMAGEVIEWER_DEVELOPMENT
#undef ImageViewerAPI
#define ImageViewerAPI
#endif
#ifdef IMAGEVIEWER_EXPORT
#undef ImageViewerAPI
#define ImageViewerAPI __declspec(dllexport)
#endif
#else
#define ImageViewerAPI
#endif
#endif // Plugin_ImageViewer_ImageViewerCompileMacro_h
