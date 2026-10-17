#ifndef Plugin_YSSFileExt_FileServer_YSSPEditor_h
#define Plugin_YSSFileExt_FileServer_YSSPEditor_h
#include <Editor/FileServer.h>
#include <Editor/EditorPlugin.h>
#include <Editor/FileEditWidget.h>
namespace YSSFileExt {
	class YSSPEditorPrivate;
	class YSSPEditor :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	public:
		YSSPEditor(QWidget* parent = nullptr);
		virtual ~YSSPEditor();
	public:
		bool onOpen(const YSSCore::General::YSSURI& uri, const QByteArray& data) override;
		bool onClose() override;
		std::expected<QByteArray, bool> onSave(const YSSCore::General::YSSURI& uri) override;
	private:
		YSSPEditorPrivate* d;
	};

	class YSSPFileServer :public YSSCore::Editor::FileServer {
	public:
		YSSPFileServer(YSSCore::Editor::EditorPlugin* plugin);
		virtual ~YSSPFileServer();
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};
}
#endif // Plugin_YSSFileExt_FileServer_YSSPEditor_h
