#pragma once
#include "../Macro.h"
#include <General/PluginModule.h>
// Forward declarations
class QString;
class QWidget;
namespace YSSCore::Editor {
	class FileEditWidget;
	class FileServerPrivate;
	class EditorPlugin;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileServer :public Visindigo::General::PluginModule{
		Q_OBJECT;
	public:
		enum EditorType {
			CodeEditor,  //builtin code editor
			BuiltInEditor, //builtin, but not code editor
		};
		FileServer(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~FileServer();
		EditorType getEditorType();
		QStringList getSupportedFileExts();
		virtual qint64 especiallyFocusFile(const QString& filePath);
		virtual FileEditWidget* onCreateFileEditWidget();
		bool isVirtualFileServer();
	protected:
		void setEditorType(EditorType type);
		void setSupportedFileExts(const QStringList& exts);
		void setAsVitrualFileServer(bool isVirtual);
	private:
		FileServerPrivate* d;
	};
}