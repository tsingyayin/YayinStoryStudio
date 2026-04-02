#include "Editor/SyntaxHighlighter.h"
#include "Editor/TextEdit.h"
#include "Editor/DocumentMessageManager.h"
#include "Editor/private/DocumentMessageManager_p.h"
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
		QUrl HelpUrl;
	};

	DocumentMessage::DocumentMessage(MessageType type, const QString& message, qint32 lineNumber,
		qint32 columnNumber, qint32 length, const QString& code, const QUrl& helpUrl) {
		d = new DocumentMessagePrivate();
		d->Type = type;
		d->Message = message;
		d->LineNumber = lineNumber;
		d->ColumnNumber = columnNumber;
		d->Length = length;
		d->Code = code;
		d->HelpUrl = helpUrl;
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

	class SyntaxHighlighterPrivate {
		friend class SyntaxHighlighter;
	protected:
		TextEdit* Parent;
	};

	SyntaxHighlighter::SyntaxHighlighter(TextEdit* parent) : QSyntaxHighlighter(parent->getDocument()) {
		d = new SyntaxHighlighterPrivate();
		d->Parent = parent;
	}

	SyntaxHighlighter::~SyntaxHighlighter() {
		DocumentMessageManager::getInstance()->d->clearMessagesForFile(d->Parent->getFilePath());
		delete d;
	}

	void SyntaxHighlighter::highlightBlock(const QString& text) {
		int blockNumber = currentBlock().blockNumber();
		DocumentMessageManager::getInstance()->d->clearMessagesForBlock(d->Parent->getFilePath() , blockNumber);
		onBlockChanged(text, blockNumber);
	}

	void SyntaxHighlighter::createErrorMessage(const QString& message, int columnNumber, int length, const QString& code, const QUrl& helpUrl) {
		DocumentMessageManager::getInstance()->d->addMessage(d->Parent->getFilePath(), currentBlock().blockNumber(),
			DocumentMessage(DocumentMessage::Error, message, currentBlock().blockNumber(), columnNumber, length, code, helpUrl));
	}

	void SyntaxHighlighter::createWarningMessage(const QString& message, int columnNumber, int length, const QString& code, const QUrl& helpUrl) {
		DocumentMessageManager::getInstance()->d->addMessage(d->Parent->getFilePath(), currentBlock().blockNumber(),
			DocumentMessage(DocumentMessage::Warning, message, currentBlock().blockNumber(), columnNumber, length, code, helpUrl));
	}

	void SyntaxHighlighter::createInfoMessage(const QString& message, int columnNumber, int length, const QString& code, const QUrl& helpUrl) {
		DocumentMessageManager::getInstance()->d->addMessage(d->Parent->getFilePath(), currentBlock().blockNumber(),
			DocumentMessage(DocumentMessage::Info, message, currentBlock().blockNumber(), columnNumber, length, code, helpUrl));
	}
}