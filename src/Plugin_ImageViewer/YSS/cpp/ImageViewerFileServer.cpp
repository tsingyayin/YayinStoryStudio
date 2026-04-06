#include "YSS/ImageViewerFileServer.h"
#include "YSS/ImageViewer.h"
namespace YSS::ImageViewer {
	FS_ImageViewer::FS_ImageViewer(YSSCore::Editor::EditorPlugin* parent) : 
		YSSCore::Editor::FileServer("ImageViewer","cn.yxgeneral.imageviewer.fs",parent) {
		setEditorType(EditorType::BuiltInEditor);
		setSupportedFileExts({ "png", "jpg", "jpeg", "bmp", "gif", "tiff", "webp" });
	}

	FS_ImageViewer::~FS_ImageViewer() {
	}

	YSSCore::Editor::FileEditWidget* FS_ImageViewer::onCreateFileEditWidget() {
		return new ImageViewer();
	}
}