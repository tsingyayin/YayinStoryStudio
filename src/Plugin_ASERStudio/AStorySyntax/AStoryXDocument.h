#ifndef ASERStudio_AStorySyntax_AStoryXDocument_h
#define ASERStudio_AStorySyntax_AStoryXDocument_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <VIMacro.h>
#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include "AStorySyntax/AStoryXRule.h"
#include <QtGui/qtextobject.h>

class QTextDocument;
namespace ASERStudio::AStorySyntax {
	class AStoryXDocumentPrivate;
	class ASERAPI AStoryXDocument :public QObject{
		Q_OBJECT;
	signals:
		void currentRuleChanged();
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
		void setEnableDiagnostic(bool enable);
		void setSyntaxHighlighter(QSyntaxHighlighter* highlighter);
		void onSyntaxHighlighter(QTextBlock currentBlock, const QString& text);
		AStoryXRule* getCurrentRule() const;
		bool isDiagnosticEnabled() const;
	private:
		AStoryXDocumentPrivate* d;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXDocument_h