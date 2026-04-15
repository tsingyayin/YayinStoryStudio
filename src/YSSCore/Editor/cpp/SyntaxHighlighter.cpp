#include "Editor/SyntaxHighlighter.h"
#include "Editor/TextEdit.h"
#include "Editor/DocumentMessageManager.h"
#include "Editor/private/DocumentMessageManager_p.h"
#include <Widgets/ThemeManager.h>
#include <General/Log.h>
#include "Editor/private/TextEdit_p.h"
#include <QtCore/qtimer.h>
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

	class SyntaxHighlighterPrivate {
		friend class SyntaxHighlighter;
	protected:
		TextEdit* Parent;
		QString FilePath;
		bool AutoRenderMessageWaveLine = true;
	};

	/*!
		\class YSSCore::Editor::SyntaxHighlighter
		\brief SyntaxHighlighter是一个基于QSyntaxHighlighter的抽象类，用于实现文本编辑器中的语法高亮和错误提示功能。
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup LangService

		SyntaxHighlighter是在YSS语言服务架构理念下强化的QSyntaxHighlighter，提供了与编辑器文本块关联的错误、
		警告和信息消息的创建和管理功能。它通过DocumentMessageManager将这些消息与文档和文本块关联起来，供编辑器界面显示和用户交互使用。

		你在这类里面仍然可以使用继承自QSyntaxHighlighter的函数，但你不能使用
		QSyntaxHighlighter::highlightBlock()，这函数已经为此类重写了，你需要重写onBlockChanged()来实现你的语法高亮和消息创建逻辑。
	*/

	/*!
		\since YSS 0.13.0
		\a parent 关联的TextEdit对象，不能为空。
	*/
	SyntaxHighlighter::SyntaxHighlighter(TextEdit* parent) : QSyntaxHighlighter(parent->getDocument()) {
		d = new SyntaxHighlighterPrivate();
		d->Parent = parent;
		d->FilePath = parent->getFilePath();
	}

	/*!
		\since YSS 0.13.0
		析构函数，清除与当前文件相关的所有消息。
	*/
	SyntaxHighlighter::~SyntaxHighlighter() {
		DocumentMessageManager::getInstance()->d->clearMessagesForFile(d->FilePath);
		delete d;
	}

	/*!
		\fn virtual void YSSCore::Editor::Syntaxhighlighter::onBlockChanged(const QString& text, int blockNumber) = 0
		\since YSS 0.13.0

		需要实现的纯虚函数，用户在其中实现具体的着色功能。语义上等价于QSyntaxHighlighter::highlightBlock
	*/
	/*!
		\since YSS 0.13.0
		重写自QSyntaxHighlighter的函数，在文本块内容改变时被调用。
		\a text 当前文本块的内容。
		这个函数会清除当前文本块的所有消息，然后调用onBlockChanged()让子类实现具体的高亮和消息创建逻辑，最后刷新当前文本块的消息显示。
	*/
	void SyntaxHighlighter::highlightBlock(const QString& text) {
		int blockNumber = currentBlock().blockNumber();
		DocumentMessageManager::getInstance()->d->clearMessagesForBlock(d->Parent->getFilePath() , blockNumber);
		onBlockChanged(text, blockNumber);
		DocumentMessageManager::getInstance()->d->flushMessages(d->FilePath, blockNumber);
	}

	/*!
		\since YSS 0.14.0
		安全的重新高亮整个文档。

		引入这个函数是为了修复QSyntaxHighlighter::rehighlight的至少两个缺陷：
		\list
		\li 1. rehighlight不能在着色触发时立即重新调用，必须在着色结束后
		从外部触发，否则会递归爆栈。而这个安全版函数内置了一个定时器，使得你
		可以在着色触发时根据需要直接调用全部重新着色函数，而它会自动延迟到
		下一个事件循环再触发（而当前这轮着色必然会在本轮事件循环完成）。
		\li 2. 由于rehighlight内部的实现方式，其在QTextCursor上的操作会
		触发QTextEdit::textChanged信号，而YSSCore::Editor::TextEdit通过监听
		此信号来感知文件是否已改变，这会导致直接调用rehighlight会使TextEdit错误
		的判断文件的改变情况。而此安全版函数会与其对应的TextEdit进行实现协商，
		使其自动屏蔽从rehighlight触发的textChanged信号。
		\endlist
	*/
	void SyntaxHighlighter::rehighlight_s() {
		if (d->Parent) {
			d->Parent->d->Rehighlighting = true;
		}
		QTimer::singleShot(0, this, [this]() {
			this->rehighlight();
			if (d->Parent) {
				d->Parent->d->Rehighlighting = false;
			}
			});
	}
	/*!
		\since YSS 0.13.0
		设置是否自动为消息创建波浪线下划线。默认为true。
		\a autoRender 是否自动渲染消息的波浪线下划线。
		如果设置为true，当你创建错误、警告或信息消息时，SyntaxHighlighter会自动在相关文本下添加波浪线下划线来提示用户。你可以通过VISTM->getColor("ErrorLine")、VISTM->getColor("WarningLine")和VISTM->getColor("InfoLine")来获取不同类型消息的下划线颜色。
		如果设置为false，你需要自己在onBlockChanged()中使用setFormat()来设置相关文本的格式来提示用户。
	*/
	void SyntaxHighlighter::autoRenderMessageWaveLine(bool autoRender) {
		d->AutoRenderMessageWaveLine = autoRender;
	}

	/*!
		\since YSS 0.13.0
		创建一个错误消息，并关联到当前文本块。你可以在onBlockChanged()中调用这个函数来为当前文本块创建错误提示。
		\a message 消息文本内容。
		\a columnNumber 消息所在的列号（从0开始），默认为-1表示未知。
		\a length 消息相关文本的长度，默认为-1表示未知。
		\a code 消息的代码标识符，默认为空字符串。
		\a helpUrl 与消息相关的帮助链接，默认为空URL。
		\a fixeAdvice 修复建议文本，默认为空字符串。
	*/
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

	/*!
		\since YSS 0.13.0
		创建一个警告消息，并关联到当前文本块。你可以在onBlockChanged()中调用这个函数来为当前文本块创建警告提示。
		\a message 消息文本内容。
		\a columnNumber 消息所在的列号（从0开始），默认为-1表示未知。
		\a length 消息相关文本的长度，默认为-1表示未知。
		\a code 消息的代码标识符，默认为空字符串。
		\a helpUrl 与消息相关的帮助链接，默认为空URL。
		\a fixeAdvice 修复建议文本，默认为空字符串。
	*/
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

	/*!
		\since YSS 0.13.0
		创建一个信息消息，并关联到当前文本块。你可以在onBlockChanged()中调用这个函数来为当前文本块创建信息提示。
		\a message 消息文本内容。
		\a columnNumber 消息所在的列号（从0开始），默认为-1表示未知。
		\a length 消息相关文本的长度，默认为-1表示未知。
		\a code 消息的代码标识符，默认为空字符串。
		\a helpUrl 与消息相关的帮助链接，默认为空URL。
		\a fixeAdvice 修复建议文本，默认为空字符串。
	*/
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