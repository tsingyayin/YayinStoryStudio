#include "FileServer/DefaultTextEdit.h"

namespace YSSFileExt {
	DefaultTextEdit::DefaultTextEdit(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("Default Text Editor", "YSSFileExt.FileServer.DefaultTextEdit", plugin) {
		setEditorType(EditorType::CodeEditor);
		setSupportedFileExts({ "txt", "json", "yaml", "yml", "gitignore", "md", "xml", "log", "ini" });
	}

	DefaultTextEdit::~DefaultTextEdit() {
	}
}