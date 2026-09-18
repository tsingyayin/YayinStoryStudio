#ifndef YayinStoryStudio_Editor_MainEditor_DocumentMessageTracer_h
#define YayinStoryStudio_Editor_MainEditor_DocumentMessageTracer_h
#include <QtCore/qobject.h>
#include <Editor/CustomDocumentData.h>
namespace YSS::Editor {
	class DocumentMessageTracer :public QObject, public YSSCore::Editor::ICustomDocumentData {
		Q_OBJECT;
	signals:
		void lineChanged(qint32 oldLine, qint32 newLine);
	public:
		DocumentMessageTracer();
		virtual ~DocumentMessageTracer();
	public:
		qint32 getShownLine() const;
		void setShownLine(qint32 line);
		qint32 getErrorCount() const;
		qint32 getWarningCount() const;
		qint32 getInfoCount() const;
		void setMessageCount(qint32 error, qint32 warning, qint32 info);
	public:
		virtual void onBlockNumberChanged(qint32 newBlockNumber) override;
	private:
		qint32 ShownLine = -1;
		qint32 ErrorCount = 0;
		qint32 WarningCount = 0;
		qint32 InfoCount = 0;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_DocumentMessageTracer_h
