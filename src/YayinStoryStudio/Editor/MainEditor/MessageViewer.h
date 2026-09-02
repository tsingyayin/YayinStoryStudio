#ifndef YayinStoryStudio_Editor_MainEditor_MessageViewer_h
#define YayinStoryStudio_Editor_MainEditor_MessageViewer_h
#include <QtWidgets/qframe.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qtablewidget.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServer.h>
namespace YSS::Editor {
	class MessageViewerVFS :public YSSCore::Editor::FileServer {
		Q_OBJECT;
	public:
		MessageViewerVFS(YSSCore::Editor::EditorPlugin* plugin);
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};

	class MessageViewer :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	private:
		QVBoxLayout* Layout;
		QTableWidget* MessageTable;
		QString CurrentFilePath;
	public:
		MessageViewer(QWidget* parent = nullptr);
		void changeCurrentFile(const QString& filePath);
	public slots:
		void onCellClicked(int row, int column);
		void onMessageChanged(const QString& filePath);
		void onMessageChangedForLine(const QString& filePath, qint32 lineNumber);
	public:
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param);
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_MessageViewer_h
