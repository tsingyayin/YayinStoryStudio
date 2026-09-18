#ifndef YayinStoryStudio_Editor_MainEditor_MessageViewer_h
#define YayinStoryStudio_Editor_MainEditor_MessageViewer_h
#include <QtWidgets/qframe.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qtablewidget.h>
#include <QtCore/qpointer.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServer.h>
#include <Editor/TextEdit.h>
#include "Editor/MainEditor/DocumentMessageTracer.h"
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
		QPointer<YSSCore::Editor::TextEdit> CurrentEdit;
	private:
		quintptr rowTracer(int row) const;
		void setRowTracer(int row, DocumentMessageTracer* tracer);
		void removeRowsForTracer(DocumentMessageTracer* tracer);
		void insertMessagesForLine(qint32 lineNumber);
		void syncMessageRows();
	public:
		MessageViewer(QWidget* parent = nullptr);
		void changeCurrentFile(YSSCore::Editor::TextEdit* edit);
	public slots:
		void onCellClicked(int row, int column);
		void onMessageChangedForLine(qint32 lineNumber);
		void onTracerLineChanged(DocumentMessageTracer* tracer, qint32 oldLine, qint32 newLine);
		void onTracerDestroyed(DocumentMessageTracer* tracer);
	public:
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param);
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_MessageViewer_h
