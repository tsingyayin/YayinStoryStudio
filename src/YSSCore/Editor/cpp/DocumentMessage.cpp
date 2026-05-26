#include "Editor/DocumentMessage.h"
#include "Editor/private/DocumentMessageManager_p.h"

namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::DocumentMessage
		\brief DocumentMessage代表文档特定位置的消息，如错误、警告或信息提示。
		\since YSS 0.13.0
		\inmodule YSSCore

		DocumentMessage是个数据类，包含消息类型、文本、位置（行号、列号和长度）、
		代码标识符、帮助链接和修复建议等信息。它被SyntaxHighlighter用于表示语法分析过程中发现的问题，并自动
		收集到YSSCore::Editor::DocumentMessageManager中，供编辑器界面显示和用户交互使用。
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
}