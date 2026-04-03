#include "Editor/SyntaxHighlighter.h"
#include "Editor/TextEdit.h"
#include "Editor/DocumentMessageManager.h"
#include "Editor/private/DocumentMessageManager_p.h"
#include <Widgets/ThemeManager.h>
#include <General/Log.h>
namespace YSSCore::Editor {
	class DocumentMessagePrivate {
		friend class DocumentMessage;
	protected:
		DocumentMessage::MessageType Type;
		QString Message;
		qint32 LineNumber;
		qint32 ColumnNumber;
		qint32 Length;
		QString Code;
		QString fixAdvice;
		QUrl HelpUrl;
	};

	DocumentMessage::DocumentMessage(MessageType type, const QString& message, qint32 lineNumber,
		qint32 columnNumber, qint32 length, const QString& code, const QUrl& helpUrl, const QString& fixeAdvice) {
		d = new DocumentMessagePrivate();
		d->Type = type;
		d->Message = message;
		d->LineNumber = lineNumber;
		d->ColumnNumber = columnNumber;
		d->Length = length;
		d->Code = code;
		d->HelpUrl = helpUrl;
		d->fixAdvice = fixeAdvice;
	}

	DocumentMessage::~DocumentMessage() {
		delete d;
	}

	VIMoveable_Impl(DocumentMessage);
	VICopyable_Impl(DocumentMessage);

	DocumentMessage::MessageType DocumentMessage::getType() const {
		return d->Type;
	}

	QString DocumentMessage::getMessage() const {
		return d->Message;
	}

	int DocumentMessage::getLineNumber() const {
		return d->LineNumber;
	}

	int DocumentMessage::getColumnNumber() const {
		return d->ColumnNumber;
	}

	int DocumentMessage::getLength() const {
		return d->Length;
	}

	QString DocumentMessage::getCode() const {
		return d->Code;
	}

	QUrl DocumentMessage::getHelpUrl() const {
		return d->HelpUrl;
	}

	QString DocumentMessage::getFixAdvice() const {
		return d->fixAdvice;
	}

	QString DocumentMessage::toString() const {
		return QString("[%1] Line %2, Column %3: %4 (Code: %5, Help: %6)")
			.arg(d->Type == MessageType::Error ? "Error" : (d->Type == MessageType::Warning ? "Warning" : "Info"))
			.arg(d->LineNumber)
			.arg(d->ColumnNumber)
			.arg(d->Message)
			.arg(d->Code)
			.arg(d->HelpUrl.toString());
	}

	class SyntaxHighlighterPrivate {
		friend class SyntaxHighlighter;
	protected:
		TextEdit* Parent;
		QString FilePath;
		bool AutoRenderMessageWaveLine = true;
	};

	SyntaxHighlighter::SyntaxHighlighter(TextEdit* parent) : QSyntaxHighlighter(parent->getDocument()) {
		d = new SyntaxHighlighterPrivate();
		d->Parent = parent;
		d->FilePath = parent->getFilePath();
	}

	SyntaxHighlighter::~SyntaxHighlighter() {
		DocumentMessageManager::getInstance()->d->clearMessagesForFile(d->FilePath);
		delete d;
	}

	void SyntaxHighlighter::highlightBlock(const QString& text) {
		int blockNumber = currentBlock().blockNumber();
		DocumentMessageManager::getInstance()->d->clearMessagesForBlock(d->Parent->getFilePath() , blockNumber);
		onBlockChanged(text, blockNumber);
		DocumentMessageManager::getInstance()->d->flushMessages(d->FilePath, blockNumber);
	}

	void SyntaxHighlighter::autoRenderMessageWaveLine(bool autoRender) {
		d->AutoRenderMessageWaveLine = autoRender;
	}

	void SyntaxHighlighter::createErrorMessage(const QString& message, int columnNumber, int length, const QString& code, const QUrl& helpUrl, const QString& fixeAdvice) {
		if (d->AutoRenderMessageWaveLine) {
			QTextBlock block = currentBlock();
			QTextCharFormat format = block.charFormat();
			format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
			format.setUnderlineColor(VISTM->getColor("ErrorLine"));
			setFormat(columnNumber, length, format);
		}
		DocumentMessageManager::getInstance()->d->addMessage(d->Parent->getFilePath(), currentBlock().blockNumber(),
			DocumentMessage(DocumentMessage::Error, message, currentBlock().blockNumber(), columnNumber, length, code, helpUrl, fixeAdvice));
	}

	void SyntaxHighlighter::createWarningMessage(const QString& message, int columnNumber, int length, const QString& code, const QUrl& helpUrl, const QString& fixeAdvice) {
		if (d->AutoRenderMessageWaveLine) {
			QTextBlock block = currentBlock();
			QTextCharFormat format = block.charFormat();
			format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
			format.setUnderlineColor(VISTM->getColor("WarningLine"));
			setFormat(columnNumber, length, format);
		}
		DocumentMessageManager::getInstance()->d->addMessage(d->Parent->getFilePath(), currentBlock().blockNumber(),
			DocumentMessage(DocumentMessage::Warning, message, currentBlock().blockNumber(), columnNumber, length, code, helpUrl, fixeAdvice));
	}

	void SyntaxHighlighter::createInfoMessage(const QString& message, int columnNumber, int length, const QString& code, const QUrl& helpUrl, const QString& fixeAdvice) {
		if (d->AutoRenderMessageWaveLine) {
			QTextBlock block = currentBlock();
			QTextCharFormat format = block.charFormat();
			format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
			format.setUnderlineColor(VISTM->getColor("InfoLine"));
			setFormat(columnNumber, length, format);
		}
		DocumentMessageManager::getInstance()->d->addMessage(d->Parent->getFilePath(), currentBlock().blockNumber(),
			DocumentMessage(DocumentMessage::Info, message, currentBlock().blockNumber(), columnNumber, length, code, helpUrl, fixeAdvice));
	}
}