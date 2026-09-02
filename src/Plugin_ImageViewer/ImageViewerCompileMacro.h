#ifndef Plugin_ImageViewer_ImageViewerCompileMacro_h
#define Plugin_ImageViewer_ImageViewerCompileMacro_h
#define ImageViewerAPI __declspec(dllimport)
#ifdef IMAGEVIEWER_DEVELOPMENT
#undef ImageViewerAPI
#define ImageViewerAPI
#endif
#ifdef IMAGEVIEWER_EXPORT
#undef ImageViewerAPI
#define ImageViewerAPI __declspec(dllexport)
#endif
#endif // Plugin_ImageViewer_ImageViewerCompileMacro_h
