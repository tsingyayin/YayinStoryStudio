#ifndef YSSCore_Editor_SyntaxHighlighter_h
#define YSSCore_Editor_SyntaxHighlighter_h
#include "YSSCoreCompileMacro.h"
#include <QtGui/qsyntaxhighlighter.h>
namespace YSSCore::__Private__ {
	class DocumentMessageManagerPrivate;
}

namespace YSSCore::Editor {
	class TextEdit;
	class SyntaxHighlighterPrivate;
	class YSSCoreAPI SyntaxHighlighter : public QSyntaxHighlighter {
		Q_OBJECT
	public:
		SyntaxHighlighter(TextEdit* parent = nullptr);
		virtual ~SyntaxHighlighter();
	private:
		void highlightBlock(const QString& text) override;
	public:
		virtual void onBlockChanged(const QString& text, int blockNumber) = 0;
		virtual void onBlockRemoved(qint32 startBlockNumber, qint32 count);
		virtual void onBlockAdded(qint32 startBlockNumber, qint32 count);
		void rehighlight_s();
		void autoRenderMessageWaveLine(bool autoRender);
		void createErrorMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
		void createWarningMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
		void createInfoMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
	private:
		SyntaxHighlighterPrivate* d;
	};
}
#endif // YSSCore_Editor_SyntaxHighlighter_h