#pragma once

#define ImageViewerAPI __declspec(dllimport)
#ifdef IMAGEVIEWER_DEVELOPMENT
#undef ImageViewerAPI
#define ImageViewerAPI
#endif
#ifdef IMAGEVIEWER_EXPORT
#undef ImageViewerAPI
#define ImageViewerAPI __declspec(dllexport)
#endif
