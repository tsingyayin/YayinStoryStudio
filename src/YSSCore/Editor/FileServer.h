#ifndef YSSCore_Editor_FileServer_h
#define YSSCore_Editor_FileServer_h
#include "YSSCoreCompileMacro.h"
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
	class YSSCoreAPI FileServer :public Visindigo::General::PluginModule {
		Q_OBJECT;
	public:
		enum EditorType {
			CodeEditor,  //builtin code editor
			BuiltInEditor, //builtin, but not code editor
		};
		enum PreferredOrientation {
			Vertical_Narrow = 0x0001,
			Vertical_Wide = 0x0002,
			Vertical = Vertical_Narrow | Vertical_Wide,
			Horizontal_Narrow = 0x0004,
			Horizontal_Wide = 0x0008,
			Horizontal = Horizontal_Narrow | Horizontal_Wide,
			Any = Vertical | Horizontal
		};
	public:
		FileServer(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~FileServer();
	public:
		EditorType getEditorType();
		QStringList getSupportedFileExts();
		bool isVirtualFileServer();
		PreferredOrientation getPreferredOrientation();
		QIcon getFileServerIcon();
		bool isListAsTool();
		QString getToolNickname();
	public:
		virtual qint64 especiallyFocusFile(const QString& filePath);
		virtual FileEditWidget* onCreateFileEditWidget();
	protected:
		void setEditorType(EditorType type);
		void setSupportedFileExts(const QStringList& exts);
		void setAsVitrualFileServer(bool isVirtual);
		void setFileServerIcon(const QIcon& icon);
		void setPreferredOrientation(PreferredOrientation orientation);
		void setListAsTool(bool  isTool);
		void setToolNickname(const QString& vi18n_name);
	private:
		FileServerPrivate* d;
	};
}
#endif // YSSCore_Editor_FileServer_h