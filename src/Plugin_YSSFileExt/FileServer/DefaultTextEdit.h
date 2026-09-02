#ifndef Plugin_YSSFileExt_FileServer_DefaultTextEdit_h
#define Plugin_YSSFileExt_FileServer_DefaultTextEdit_h
#include <Editor/FileServer.h>
#include <Editor/EditorPlugin.h>
#include <Editor/FileEditWidget.h>

namespace YSSFileExt {
	class DefaultTextEdit :public YSSCore::Editor::FileServer {
	public:
		DefaultTextEdit(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~DefaultTextEdit();
	};
}

#endif // Plugin_YSSFileExt_FileServer_DefaultTextEdit_h
