#include "Editor/SyntaxHighlighter.h"
#include "Editor/TextEdit.h"
#include "Editor/DocumentMessageManager.h"
#include "Editor/private/DocumentMessageManager_p.h"
#include <Widgets/ThemeManager.h>
#include <General/Log.h>
#include "Editor/private/TextEdit_p.h"
#include <QtCore/qtimer.h>
namespace YSSCore::Editor {
	class SyntaxHighlighterPrivate {
		friend class SyntaxHighlighter;
	protected:
		TextEdit* Parent;
		QString FilePath;
		bool AutoRenderMessageWaveLine = true;
		qint32 LastTotalBlockNumber = 0;
	};

	/*!
		\class YSSCore::Editor::SyntaxHighlighter
		\brief SyntaxHighlighter是一个基于QSyntaxHighlighter的抽象类，用于实现文本编辑器中的语法高亮和错误提示功能.
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
		\fn YSSCore::Editor::SyntaxHighlighter::onBlockChanged(const QString& text, int blockNumber)
		\since YSS 0.13.0
		\a text 当前文本块的内容，
		\a blockNumber 当前文本块的块号。

		需要实现的纯虚函数，用户在其中实现具体的着色功能。语义上等价于QSyntaxHighlighter::highlightBlock
	*/

	/*!
		\since YSS 0.15.0
		\a startBlockNumber 被删除的文本块的起始块号， \a count 被删除的文本块的数量。

		当每次触发着色时，如果当前文本块的总数小于上次触发时的总数，就会调用这个函数。默认实现为空。
		这个函数先于onBlockChanged()被调用。
	*/
	void SyntaxHighlighter::onBlockRemoved(qint32 startBlockNumber, qint32 count) {
	}

	/*!
		\since YSS 0.15.0
		\a startBlockNumber 新增的文本块的起始块号， \a count 新增的文本块的数量。

		当每次触发着色时，如果当前文本块的总数大于上次触发时的总数，就会调用这个函数。默认实现为空。
		这个函数先于onBlockChanged()被调用。
	*/
	void SyntaxHighlighter::onBlockAdded(qint32 startBlockNumber, qint32 count) {}

	/*!
		\since YSS 0.13.0
		重写自QSyntaxHighlighter的函数，在文本块内容改变时被调用。
		\a text 当前文本块的内容。

		这个函数会清除当前文本块的所有消息，然后调用onBlockChanged()让子类实现具体的高亮和消息创建逻辑，最后刷新当前文本块的消息显示。
		它也负责根据行数的变化自动前后移动消息，以保持消息与文本块的正确关联。
	*/
	void SyntaxHighlighter::highlightBlock(const QString& text) {
		qint32 totalNumber = currentBlock().document()->blockCount();
		if (totalNumber < d->LastTotalBlockNumber) {
			qint32 removedCount = d->LastTotalBlockNumber - totalNumber;
			qint32 startBlockNumber = currentBlock().blockNumber();
			//vgDebug << "Blocks removed: " << removedCount << ", from " << startBlockNumber << " to " << startBlockNumber + removedCount - 1;
			DocumentMessageManager::getInstance()->d->moveMessageForward(d->Parent->getFilePath(), startBlockNumber, removedCount);
			onBlockRemoved(startBlockNumber, removedCount);
		}
		else if (totalNumber > d->LastTotalBlockNumber) {
			qint32 addedCount = totalNumber - d->LastTotalBlockNumber;
			qint32 startBlockNumber = currentBlock().blockNumber();
			//vgDebug << "Blocks added: " << addedCount << ", from " << startBlockNumber << " to " << startBlockNumber + addedCount - 1;
			DocumentMessageManager::getInstance()->d->moveMessageBackward(d->Parent->getFilePath(), startBlockNumber, addedCount);
			onBlockAdded(startBlockNumber, addedCount);
		}
		d->LastTotalBlockNumber = totalNumber;
		int blockNumber = currentBlock().blockNumber();
		DocumentMessageManager::getInstance()->d->clearMessagesForBlock(d->Parent->getFilePath(), blockNumber);
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