#ifndef YSSCore_Editor_SyntaxHighlighter_h
#define YSSCore_Editor_SyntaxHighlighter_h
#include "YSSCoreCompileMacro.h"
#include <QtGui/qsyntaxhighlighter.h>
#include "Editor/InlayHint.h"
namespace YSSCore::Editor {
	class StyleData;
	class TextEdit;
	class SyntaxHighlighterPrivate;
	class YSSCoreAPI SyntaxHighlighter : public QSyntaxHighlighter {
		Q_OBJECT
		friend class SyntaxHighlighterPrivate;
	signals:
		void inlayHintChangedForLine(qint32 blockNumber);
		void wholeDocumentHighlighted();
	public:
		SyntaxHighlighter(TextEdit* parent = nullptr);
		virtual ~SyntaxHighlighter();
	private:
		void highlightBlock(const QString& text) override;
		void addInlayHint(qint32 blockNumber, const InlayHint& hint);
		void clearInlayHints(qint32 blockNumber);
		void applyInlayHintSpacings(const QString& text, qint32 blockNumber);
	public:
		virtual void onBlockChanged(const QString& text, int blockNumber) = 0;
		virtual void onBlockRemoved(qint32 startBlockNumber, qint32 count);
		virtual void onBlockAdded(qint32 startBlockNumber, qint32 count);
		virtual void onThemeChanged(const QMap<QString, StyleData>& theme);
	protected:
		void setCharFormat(qint32 start, qint32 count, const QTextCharFormat& format);
	public:
		void rehighlight_s();
		void autoRenderMessageWaveLine(bool autoRender);
		void createErrorMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
		void createWarningMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
		void createInfoMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
	public:
		void createInlayHint(const InlayHint& hint);
		void createInlayHint(qint32 positionInBlock, const QString& text, const QColor& color = QColor(), qint32 paddings = 8);
		QList<InlayHint> getInlayHints(qint32 blockNumber) const;
		QMap<qint32, QList<InlayHint>> getAllInlayHints() const;
		void setAutoRenderInlayHint(bool autoRender);
		bool isAutoRenderInlayHint() const;
	private:
		SyntaxHighlighterPrivate* d;
	};
}
#endif // YSSCore_Editor_SyntaxHighlighter_h