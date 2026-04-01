#ifndef ASERStudio_AStorySyntax_AStoryXDocument_h
#define ASERStudio_AStorySyntax_AStoryXDocument_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <VIMacro.h>
#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include "AStorySyntax/AStoryXRule.h"

class QTextDocument;
namespace ASERStudio::AStorySyntax {
	class AStoryXDocumentPrivate;
	class ASERAPI AStoryXDocument :public QObject{
		Q_OBJECT;
	signals:
		void parseDataUpdated(qint32 lineNumber);
	public:
		AStoryXDocument();
		~AStoryXDocument();
		QStringList getLines() const;
		QList<AStoryXDiagnosticData> getDiagnostics(qint32 lineNumber) const;
		QList<AStoryXDiagnosticData> getAllDiagnostics() const;
		QList<AStoryXDiagnosticData> getGlobalDiagnostics() const;
		AStoryXControllerParseData getParseData(qint32 lineNumber) const;
		QList<AStoryXControllerParseData> getAllParseData() const;
		void setTextDocument(QTextDocument* doc);
		QTextDocument* getTextDocument() const;
		void addRule(AStoryXRule* rule);
		void setEnableDiagnostic(bool enable);
		bool isDiagnosticEnabled() const;
	private:
		AStoryXDocumentPrivate* d;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXDocument_h