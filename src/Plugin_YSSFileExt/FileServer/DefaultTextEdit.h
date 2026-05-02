#ifndef YSSFileExt_LangServer_DefaultTextEdit_H
#define YSSFileExt_LangServer_DefaultTextEdit_H
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

#endif // YSSFileExt_LangServer_DefaultTextEdit_H
