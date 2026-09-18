#include "Editor/DocumentMessage.h"
#include "Editor/TextEdit.h"
#include "Editor/private/DocumentMessage_p.h"
#include "Editor/private/TextEdit_p.h"

namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::DocumentMessage
		\brief DocumentMessage代表文档特定位置的消息，如错误、警告或信息提示.
		\since YSS 0.13.0
		\inmodule YSSCore

		DocumentMessage是个数据类，包含消息类型、文本、位置（行号、列号和长度）、
		代码标识符、帮助链接和修复建议等信息。它被SyntaxHighlighter用于表示语法分析过程中发现的问题，
		并按行存放在所属的YSSCore::Editor::TextEdit上，供编辑器界面显示和用户交互使用。
	*/

	/*!
		\enum YSSCore::Editor::DocumentMessage::MessageType
		\since YSS 0.13.0
		\value Error 错误消息，表示代码中的错误。
		\value Warning 警告消息，表示代码中的潜在问题或不推荐的用法。
		\value Info 信息消息，表示代码中的提示或建议。
	*/

	/*!
		\brief 构造一个DocumentMessage对象。
		\a type 消息类型，如错误、警告或信息。
		\a message 消息文本内容。
		\a lineNumber 消息所在的行号（从0开始），默认为-1表示未知。
		\a columnNumber 消息所在的列号（从0开始），默认为-1表示未知。
		\a length 消息相关文本的长度，默认为-1表示未知。
		\a code 消息的代码标识符，默认为空字符串。
		\a helpUrl 与消息相关的帮助链接，默认为空URL。
		\a fixeAdvice 修复建议文本，默认为空字符串。
	*/
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

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	DocumentMessage::~DocumentMessage() {
		delete d;
	}

	/*!
		\fn YSSCore::Editor::DocumentMessage::DocumentMessage(DocumentMessage&& other) noexcept
		移动构造函数
	*/

	/*!
		\fn YSSCore::Editor::DocumentMessage& YSSCore::Editor::DocumentMessage::operator=(DocumentMessage&& other) noexcept
		移动赋值运算符

		\a other 要移动的源对象。
	*/

	/*!
		\fn YSSCore::Editor::DocumentMessage::DocumentMessage(const DocumentMessage& other)
		复制构造函数
	*/

	/*!
		\fn YSSCore::Editor::DocumentMessage& YSSCore::Editor::DocumentMessage::operator=(const DocumentMessage& other)
		复制赋值运算符
	*/
	VIMoveable_Impl(DocumentMessage);
	VICopyable_Impl(DocumentMessage);

	/*!
		\since YSS 0.13.0
		获取消息类型。
	*/
	DocumentMessage::MessageType DocumentMessage::getType() const {
		return d->Type;
	}

	/*!
		\since YSS 0.13.0
		获取消息文本内容。
	*/
	QString DocumentMessage::getMessage() const {
		return d->Message;
	}

	/*!
		\since YSS 0.13.0
		获取消息所在的行号（从0开始）。如果行号未知，则返回-1。
	*/
	int DocumentMessage::getLineNumber() const {
		return d->LineNumber;
	}

	/*!
		\since YSS 0.13.0
		获取消息所在的列号（从0开始）。如果列号未知，则返回-1。
	*/
	int DocumentMessage::getColumnNumber() const {
		return d->ColumnNumber;
	}

	/*!
		\since YSS 0.13.0
		获取消息相关文本的长度。如果长度未知，则返回-1。
	*/
	int DocumentMessage::getLength() const {
		return d->Length;
	}

	/*!
		\since YSS 0.13.0
		获取消息的代码标识符。如果没有代码标识符，则返回空字符串。
	*/
	QString DocumentMessage::getCode() const {
		return d->Code;
	}

	/*!
		\since YSS 0.13.0
		获取与消息相关的帮助链接。如果没有帮助链接，则返回空URL。
	*/
	QUrl DocumentMessage::getHelpUrl() const {
		return d->HelpUrl;
	}

	/*!
		\since YSS 0.13.0
		获取修复建议文本。如果没有修复建议，则返回空字符串。
	*/
	QString DocumentMessage::getFixAdvice() const {
		return d->fixAdvice;
	}

	/*!
		\since YSS 0.13.0
		将DocumentMessage对象转换为字符串表示形式，包含消息类型、位置、文本、代码和帮助链接等信息。
		这是个辅助调试函数。
	*/
	QString DocumentMessage::toString() const {
		return QString("[%1] Line %2, Column %3: %4 (Code: %5, Help: %6)")
			.arg(d->Type == MessageType::Error ? "Error" : (d->Type == MessageType::Warning ? "Warning" : "Info"))
			.arg(d->LineNumber)
			.arg(d->ColumnNumber)
			.arg(d->Message)
			.arg(d->Code)
			.arg(d->HelpUrl.toString());
	}

	/*!
		\fn void TextEdit::clearMessages(qint32 lineNumber)
		\since YSS 0.17.0
		\a lineNumber 行号，从 0 开始。

		清空指定行上的消息。语法高亮器会在重新着色一行之前调用它，因此一行上的消息总是当前这次解析的结果。
	*/
	void TextEdit::clearMessages(qint32 lineNumber) {
		__Private__::DocumentMessageList* list = getBlockData<__Private__::DocumentMessageList>(lineNumber);
		if (list == nullptr) {
			return;
		}
		list->Messages.clear();
		emit messageChangedForLine(lineNumber);
	}

	/*!
		\fn void TextEdit::addMessage(qint32 lineNumber, const DocumentMessage& message)
		\since YSS 0.17.0
		\a lineNumber 行号，从 0 开始。
		\a message 要添加的消息。

		给指定行添加一条消息。它一般由语法高亮器在着色过程中调用，消息会随该行的文本块一起移动。
	*/
	void TextEdit::addMessage(qint32 lineNumber, const DocumentMessage& message) {
		if (lineNumber < 0) {
			return;
		}
		__Private__::DocumentMessageList* list = getBlockData<__Private__::DocumentMessageList>(lineNumber);
		if (list == nullptr) {
			list = new __Private__::DocumentMessageList();
			setBlockData<__Private__::DocumentMessageList>(lineNumber, list);
		}
		list->Messages.append(message);
		emit messageChangedForLine(lineNumber);
	}

	/*!
		\fn bool TextEdit::hasMessage() const
		\since YSS 0.17.0
		\return 文档中是否有消息。
	*/
	bool TextEdit::hasMessage() const {
		return not getAllMessages().isEmpty();
	}

	/*!
		\fn bool TextEdit::hasMessage(qint32 lineNumber) const
		\since YSS 0.17.0
		\a lineNumber 行号，从 0 开始。

		\return 该行上是否有消息。
	*/
	bool TextEdit::hasMessage(qint32 lineNumber) const {
		__Private__::DocumentMessageList* list = getBlockData<__Private__::DocumentMessageList>(lineNumber);
		return list != nullptr && not list->Messages.isEmpty();
	}

	/*!
		\fn QList<DocumentMessage> TextEdit::getMessages(qint32 lineNumber) const
		\since YSS 0.17.0
		\a lineNumber 行号，从 0 开始。

		\return 该行上的消息。该行没有消息时返回空列表。

		返回的消息里的行号总是当前的行号，可以直接用于显示。
	*/
	QList<DocumentMessage> TextEdit::getMessages(qint32 lineNumber) const {
		__Private__::DocumentMessageList* list = getBlockData<__Private__::DocumentMessageList>(lineNumber);
		if (list == nullptr) {
			return QList<DocumentMessage>();
		}
		QList<DocumentMessage> result;
		for (DocumentMessage message : list->Messages) {
			message.d->LineNumber = lineNumber;
			result.append(message);
		}
		return result;
	}

	/*!
		\fn QMap<qint32, QList<DocumentMessage>> TextEdit::getAllMessages() const
		\since YSS 0.17.0
		\return 文档中所有需要显示的消息，以行号为键。

		只有确实带有消息的行才会出现在结果里。
	*/
	QMap<qint32, QList<DocumentMessage>> TextEdit::getAllMessages() const {
		QMap<qint32, QList<DocumentMessage>> result;
		const QMap<qint32, __Private__::DocumentMessageList*> allData = getAllBlockData<__Private__::DocumentMessageList>();
		for (auto it = allData.begin(); it != allData.end(); ++it) {
			__Private__::DocumentMessageList* list = it.value();
			if (list->Messages.isEmpty()) {
				continue;
			}
			QList<DocumentMessage> messages;
			for (DocumentMessage message : list->Messages) {
				message.d->LineNumber = it.key();
				messages.append(message);
			}
			result.insert(it.key(), messages);
		}
		return result;
	}

	/*!
		\fn std::tuple<qint32, qint32, qint32> TextEdit::getMessageCount()
		\since YSS 0.17.0
		\return 文档中错误、警告、信息三类消息的数量，顺序为错误、警告、信息。

		每次调用都会遍历整个文档，频繁取值时请调用方自行缓存。
	*/
	std::tuple<qint32, qint32, qint32> TextEdit::getMessageCount() {
		qint32 errorCount = 0;
		qint32 warningCount = 0;
		qint32 infoCount = 0;
		const QMap<qint32, QList<DocumentMessage>> all = getAllMessages();
		for (auto it = all.begin(); it != all.end(); ++it) {
			for (const DocumentMessage& message : it.value()) {
				switch (message.getType()) {
				case DocumentMessage::MessageType::Error:
					errorCount++;
					break;
				case DocumentMessage::MessageType::Warning:
					warningCount++;
					break;
				case DocumentMessage::MessageType::Info:
					infoCount++;
					break;
				}
			}
		}
		return { errorCount, warningCount, infoCount };
	}
}