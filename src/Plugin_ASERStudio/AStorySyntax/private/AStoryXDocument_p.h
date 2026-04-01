#ifndef ASERStudio_AStorySyntax_AStoryXDocumentPrivate_h
#define ASERStudio_AStorySyntax_AStoryXDocumentPrivate_h
#include <QtCore/qobject.h>
#include <QtGui/qsyntaxhighlighter.h>
namespace ASERStudio::AStorySyntax {
	class AStoryXDocumentNotifier :public QSyntaxHighlighter {
		Q_OBJECT;
	signals:
		void blockChanged(const QString& text);
	public:
		AStoryXDocumentNotifier(QObject* parent);
		virtual ~AStoryXDocumentNotifier();
		virtual void highlightBlock(const QString& text) override;
		QTextBlock getCurrentBlock() const;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXDocumentPrivate_h