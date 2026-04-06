#ifndef ImageViewer_YSS_ImageViewerFileServer_H
#define ImageViewer_YSS_ImageViewerFileServer_H
#include <Editor/FileServer.h>
#include "ImageViewerCompileMacro.h"
namespace YSS::ImageViewer {
	class ImageViewerAPI FS_ImageViewer :public YSSCore::Editor::FileServer {
	public:
		FS_ImageViewer(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~FS_ImageViewer();
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};
}
#endif // ImageViewer_YSS_ImageViewerFileServer_H
