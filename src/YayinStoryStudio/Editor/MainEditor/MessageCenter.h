#ifndef YayinStoryStudio_Editor_MainEditor_MessageCenter_h
#define YayinStoryStudio_Editor_MainEditor_MessageCenter_h
#include <tuple>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtCore/qset.h>
#include <Editor/FileEditWidget.h>
#include <Editor/TextEdit.h>
namespace YSS::Editor {
	class DocumentMessageTracer;
	class MessageCenter :public QObject {
		Q_OBJECT;
		friend class DocumentMessageTracer;
	private:
		MessageCenter();
	public:
		virtual ~MessageCenter();
		static MessageCenter* getInstance();
	public:
		YSSCore::Editor::TextEdit* getCurrentEdit() const;
		std::tuple<qint32, qint32, qint32> getMessageCount() const;
		DocumentMessageTracer* getTracer(qint32 lineNumber) const;
		QList<DocumentMessageTracer*> getTracers() const;
	signals:
		void currentEditChanged(YSSCore::Editor::TextEdit* edit);
		void messageChangedForLine(qint32 lineNumber);
		void messageCountChanged();
		void tracerLineChanged(DocumentMessageTracer* tracer, qint32 oldLine, qint32 newLine);
		void tracerDestroyed(DocumentMessageTracer* tracer);
	private slots:
		void onCurrentFileEditWidgetChanged(YSSCore::Editor::FileEditWidget* widget);
		void onMessageChanged();
		void onMessageChangedForLine(qint32 lineNumber);
	private:
		DocumentMessageTracer* ensureTracer(qint32 lineNumber);
		void applyLineCount(DocumentMessageTracer* tracer, qint32 error, qint32 warning, qint32 info);
		void onTracerLineChanged(DocumentMessageTracer* tracer, qint32 oldLine, qint32 newLine);
		void onTracerDestroyed(DocumentMessageTracer* tracer);
	private:
		static MessageCenter* Instance;
		QPointer<YSSCore::Editor::TextEdit> CurrentEdit;
		QSet<DocumentMessageTracer*> Tracers;
		std::tuple<qint32, qint32, qint32> Count = { 0, 0, 0 };
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_MessageCenter_h
