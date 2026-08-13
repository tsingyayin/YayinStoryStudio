#include "FileServer/DefaultTextEdit.h"

namespace YSSFileExt {
	DefaultTextEdit::DefaultTextEdit(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("Default Text Editor", "YSSFileExt.FileServer.DefaultTextEdit", plugin) {
		setEditorType(EditorType::CodeEditor);
		setSupportedFileExts({
			// 纯文本 / 日志
			"txt", "log", "nfo",
			// 标记语言 / 文档
			"md", "markdown", "rst", "adoc", "asciidoc", "org", "textile", "wiki",
			// 标记 / 网页
			"xml", "html", "htm", "xhtml", "css", "svg", "xsl", "xslt", "dtd",
			// 数据 / 表格
			"json", "json5", "jsonc", "yaml", "yml", "toml", "csv", "tsv",
			// 配置文件
			"ini", "cfg", "conf", "config", "properties", "env", "editorconfig", "gitignore", "gitattributes",
			// 脚本 / 代码
			"py", "js", "mjs", "cjs", "ts", "tsx", "jsx", "java", "c", "h", "cpp", "hpp", "cxx", "cs", "go", "rs",
			"sql", "sh", "bash", "bat", "cmd", "ps1", "psm1", "vbs",
			// 版本控制 / 差异
			"diff", "patch",
			// 其他常见文本格式
			"tex", "srt", "vtt", "ics", "manifest", "lock", "makefile", "dockerfile"
		});
	}

	DefaultTextEdit::~DefaultTextEdit() {}
}