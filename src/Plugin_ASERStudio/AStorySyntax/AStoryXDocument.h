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
	class ASERAPI AStoryXDocument :public QObject {
		Q_OBJECT;
	signals:
		void currentRuleChanged();
	public:
		enum WorkMode {
			SyntaxHighlighter,
			Manual,
		};
	public:
		AStoryXDocument();
		~AStoryXDocument();
		QStringList getLines() const;
		QList<AStoryXDiagnosticData> getDiagnostics(qint32 lineNumber) const;
		QList<AStoryXDiagnosticData> getAllDiagnostics() const;
		QList<AStoryXDiagnosticData> getGlobalDiagnostics() const;
		AStoryXControllerParseData getParseData(qint32 lineNumber) const;
		QList<AStoryXControllerParseData> getAllParseData() const;
		void setEnableDiagnostic(bool enable);
		void onSyntaxHighlighter(QTextBlock currentBlock, const QString& text);
		void onManualParse(qint32 lineIndex, const QString& text, qint32 cursorPosition = -1);
		void onLinesAdded(qint32 startLine, qint32 count);
		void onLinesRemoved(qint32 startLine, qint32 count);
		AStoryXRule* getCurrentRule() const;
		bool isDiagnosticEnabled() const;
		WorkMode getWorkMode() const;
	private:
		AStoryXDocumentPrivate* d;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXDocument_h