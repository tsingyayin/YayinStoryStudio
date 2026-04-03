#ifndef YSSCore_Editor_SyntaxHighlighter_h
#define YSSCore_Editor_SyntaxHighlighter_h
#include "../Macro.h"
#include <QtGui/qsyntaxhighlighter.h>

namespace YSSCore::Editor {
	class TextEdit;
	class DocumentMessagePrivate;
	class YSSCoreAPI DocumentMessage {
	public:
		enum MessageType {
			Error = 0,
			Warning = 1,
			Info = 2
		};
	public:
		DocumentMessage(MessageType type, const QString& message, qint32 lineNumber = -1,
			qint32 columnNumber = -1, qint32 length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(),
			const QString& fixAdvice = "");
		VIMoveable(DocumentMessage);
		VICopyable(DocumentMessage);
		~DocumentMessage();
	public:
		MessageType getType() const;
		QString getMessage() const;
		int getLineNumber() const;
		int getColumnNumber() const;
		int getLength() const;
		QString getCode() const;
		QUrl getHelpUrl() const;
		QString getFixAdvice() const;
		QString toString() const;
	private:
		DocumentMessagePrivate* d;
	};

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
		void autoRenderMessageWaveLine(bool autoRender);
		void createErrorMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
		void createWarningMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
		void createInfoMessage(const QString& message, int columnNumber = -1, int length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(), const QString& fixeAdvice = "");
	private:
		SyntaxHighlighterPrivate* d;
	};
}
#endif // YSSCore_Editor_SyntaxHighlighter_h