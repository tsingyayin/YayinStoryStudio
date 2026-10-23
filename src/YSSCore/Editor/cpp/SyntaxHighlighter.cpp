#include <QtCore/qtimer.h>
#include <QtCore/qelapsedtimer.h>
#include <General/Log.h>
#include <Widgets/ThemeManager.h>
#include "Editor/DocumentMessage.h"
#include "Editor/private/InlayHint_p.h"
#include "Editor/private/TextEdit_p.h"
#include "Editor/SyntaxHighlighter.h"
#include "Editor/TextEdit.h"
namespace YSSCore::Editor {
	// 视口优先着色：可见块上下各留一些余量，轻微滚动也不会看到还没着色的行。
	static constexpr qint32 ViewportHighlightOverscan = 64;
	static constexpr qint32 ViewportHighlightCount = 256;
	// 分片着色期间刷新缩略图的间隔，避免大文件补齐那几秒里缩略图一直空着。
	static constexpr qint64 ProgressiveOverviewIntervalMs = 400;
	class SyntaxHighlighterPrivate {
		friend class SyntaxHighlighter;
	protected:
		TextEdit* Parent = nullptr;
		SyntaxHighlighter* q = nullptr;
		bool AutoRenderMessageWaveLine = true;
		bool AutoRenderInlayHint = true;
		qint32 LastTotalBlockNumber = 0;
		// 批量装载内容时挂起着色，避免 QSyntaxHighlighter 对每一块都做无意义的回调。
		bool Suspended = false;
		// 分片着色：-1 表示当前没有进行中的任务。
		qint32 ProgressiveNext = -1;
		qint32 ProgressiveSkipFrom = 0;
		qint32 ProgressiveSkipTo = 0;
		qint32 ProgressiveSliceMs = 16;
		qint64 ProgressiveElapsedMs = 0;
	protected:
		void doHighlightBlock(const QTextBlock& block, const QString& text);
		void stepProgressiveHighlight();
	};

	/*!
		\class YSSCore::Editor::SyntaxHighlighter
		\brief SyntaxHighlighter是一个基于QSyntaxHighlighter的抽象类，用于实现文本编辑器中的语法高亮和错误提示功能.
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup LangService

		SyntaxHighlighter是在YSS语言服务架构理念下强化的QSyntaxHighlighter，提供了与编辑器文本块关联的错误、
		警告和信息消息的创建和管理功能。它把这些消息按行存放在宿主编辑器的文本块上，供编辑器界面显示和用户交互使用。

		你在这类里面仍然可以使用继承自QSyntaxHighlighter的函数，但你不能使用
		QSyntaxHighlighter::highlightBlock()，这函数已经为此类重写了，你需要重写onBlockChanged()来实现你的语法高亮和消息创建逻辑。
	*/

	/*!
		\since YSS 0.13.0
		\a parent 关联的TextEdit对象，不能为空。
	*/
	SyntaxHighlighter::SyntaxHighlighter(TextEdit* parent) : QSyntaxHighlighter(parent->getDocument()) {
		d = new SyntaxHighlighterPrivate();
		d->q = this;
		d->Parent = parent;
	}

	/*!
		\since YSS 0.13.0
		析构函数。
	*/
	SyntaxHighlighter::~SyntaxHighlighter() {
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
		\since YSS 0.16.0
		\a theme 新的样式主题。

		当主题改变时被调用。默认实现为空。

		这个函数与YSSCore::Editor::ColorThemeProvider::currentThemeChanged()信号不同，该信号只在主题整体变化时发出，
		而这个函数在主题内任何设置数据发生变化时都会被调用。不过它也与YSSCore::Editor::ColorThemeProvider::themeModified()信号不同，
		这个函数还有可能在用户编辑主题过程中被调用，而themeModified()信号只在用户保存主题后才会发出。

		因此，如果需要正常支持YSS的主题编辑功能，应直接在这里实现获取数据的逻辑，连接themeModified()信号可能会导致主题编辑时的实时预览无法正常工作。

		与此同时，这个函数在SyntaxHighlighter对象构造后，第一次着色调前被至少调用一次，因此无需单独编写初始获得数据的逻辑。

		除了上述的第一次调用之外，其余情况下此函数被调用后，还会自动调用rehighlight_s()来重新着色整个文档，因此你无需在这里手动调用rehighlight_s()。
	*/
	void SyntaxHighlighter::onThemeChanged(const QMap<QString, StyleData>& theme) {}

	/*!
		\since YSS 0.13.0
		重写自QSyntaxHighlighter的函数，在文本块内容改变时被调用。
		\a text 当前文本块的内容。

		这个函数会清除当前文本块的所有消息和所有内联提示，然后调用onBlockChanged()让子类实现具体的高亮和消息创建逻辑，
		最后刷新当前文本块的消息显示，并为这轮解析出的内联提示注入空隙。
		它也负责根据行数的变化自动前后移动消息，以保持消息与文本块的正确关联。
	*/
	void SyntaxHighlighter::highlightBlock(const QString& text) {
		d->doHighlightBlock(currentBlock(), text);
	}

	/*!
		\since YSS 0.18.0
		\a block 要着色的文本块。
		\a text 该文本块的内容。

		单个文本块的着色流程。QSyntaxHighlighter 的 highlightBlock 与 rehighlightBlock 都走这里。
	*/
	void SyntaxHighlighterPrivate::doHighlightBlock(const QTextBlock& block, const QString& text) {
		if (!block.isValid()) {
			return;
		}
		const qint32 blockNumber = block.blockNumber();
		qint32 totalNumber = block.document()->blockCount();
		if (Suspended) {
			// 挂起期间只同步块数，避免恢复后把整篇当成"新增块"再通知一遍。
			LastTotalBlockNumber = totalNumber;
			return;
		}
		if (totalNumber < LastTotalBlockNumber) {
			q->onBlockRemoved(blockNumber, LastTotalBlockNumber - totalNumber);
		}
		else if (totalNumber > LastTotalBlockNumber) {
			q->onBlockAdded(blockNumber, totalNumber - LastTotalBlockNumber);
		}
		LastTotalBlockNumber = totalNumber;
		Parent->clearMessages(blockNumber);
		q->clearInlayHints(blockNumber);
		q->onBlockChanged(text, blockNumber);
		// 提示的空隙在子类着色完成后注入：子类可能对锚点字符设过格式，而 setFormat 是覆盖语义。
		q->applyInlayHintSpacings(text, blockNumber);
	}

	void SyntaxHighlighter::applyInlayHintSpacings(const QString& text, qint32 blockNumber) {
		if (not d->AutoRenderInlayHint or not d->Parent->isInlayHintVisible()) {
			return;
		}
		const QList<InlayHint> hints = getInlayHints(blockNumber);
		if (hints.isEmpty()) {
			return;
		}
		const QFont hintFont = __Private__::inlayHintFont(d->Parent->getDocument()->defaultFont(),
			d->Parent->getInlayHintFontScale());
		for (const InlayHint& hint : hints) {
			const qint32 anchor = hint.getPositionInBlock() - 1;
			if (anchor < 0 or anchor >= text.size()) {
				// 行首没有可用的锚点字符，这类提示无法推开后续文字，绘制端也会跳过它。
				continue;
			}
			QTextCharFormat format = this->format(anchor);
			format.setFontLetterSpacingType(QFont::AbsoluteSpacing);
			format.setFontLetterSpacing(__Private__::inlayHintGapWidth(hint, hintFont));
			setFormat(anchor, 1, format);
		}
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
		触发QPlainTextEdit::textChanged信号，而YSSCore::Editor::TextEdit通过监听
		此信号来感知文件是否已改变，这会导致直接调用rehighlight会使TextEdit错误
		的判断文件的改变情况。而此安全版函数会与其对应的TextEdit进行实现协商，
		使其自动屏蔽从rehighlight触发的textChanged信号。
		\endlist
	*/
	void SyntaxHighlighter::rehighlight_s() {
		QTimer::singleShot(0, this, [this]() {
			d->Parent->d->Rehighlighting = true;
			this->rehighlight();
			d->Parent->d->Rehighlighting = false;
			emit wholeDocumentHighlighted();
			});
	}

	/*!
		\since YSS 0.18.0
		\a startBlockNumber 起始块号，从0开始。
		\a count 要重新着色的块数量，传入负数表示一直到文档末尾。

		只重新着色指定范围的块，其余语义与 rehighlight_s() 相同：延迟到下一轮事件循环、
		期间屏蔽着色本身产生的 textChanged，结束后发出 wholeDocumentHighlighted()。

		用于"变化只影响部分行"的场合——例如语法规则在文档开头才确定，那么规则生效位置
		之后的行在同一趟格式化里就已经用新规则解析过了，没有理由再解析一遍。
	*/
	void SyntaxHighlighter::rehighlightRange_s(qint32 startBlockNumber, qint32 count) {
		QTimer::singleShot(0, this, [this, startBlockNumber, count]() {
			QTextDocument* doc = this->document();
			if (doc == nullptr) {
				return;
			}
			d->Parent->d->Rehighlighting = true;
			QTextBlock block = doc->findBlockByNumber(qMax<qint32>(0, startBlockNumber));
			for (qint32 i = 0; block.isValid() && (count < 0 || i < count); ++i, block = block.next()) {
				this->rehighlightBlock(block);
			}
			d->Parent->d->Rehighlighting = false;
			// 分片着色进行中时不要提前通知：此时格式还不完整，缩略图会拿到半张空数据。
			if (d->ProgressiveNext < 0) {
				emit wholeDocumentHighlighted();
			}
			});
	}

	/*!
		\since YSS 0.18.0
		\a suspended 为true时，着色回调直接返回。

		用于批量替换文档内容的场合（例如打开一个新文件时的setPlainText）：QSyntaxHighlighter
		会对内容变化范围内的每一块同步回调一次，而此时既没有窗口可显示、也没有必要逐块着色。
		挂起只是省掉这些回调，格式本身由 QSyntaxHighlighter 在这一轮里清空，因此恢复后需要
		自己驱动一次着色（见 rehighlightProgressive_s() 或 rehighlight_s()）。
	*/
	void SyntaxHighlighter::setHighlightSuspended(bool suspended) {
		d->Suspended = suspended;
	}

	/*!
		\since YSS 0.18.0
		return 当前是否挂着色。
	*/
	bool SyntaxHighlighter::isHighlightSuspended() const {
		return d->Suspended;
	}

	/*!
		\since YSS 0.18.0
		\a sliceMilliseconds 每一片的时间预算，默认8毫秒。

		视口优先的整篇着色：先把视口附近的一批文本块同步着色（这样首帧画出来就是带色的），
		其余部分在随后的定时器里按时间片逐段补完，补完后发出 wholeDocumentHighlighted()。

		与 rehighlight_s() 的区别只在于着色顺序和调度方式，最终结果完全一致。大文件打开时，
		它让"能看到正确的颜色"这件事不必等整篇算完。
	*/
	void SyntaxHighlighter::rehighlightProgressive_s(qint32 sliceMilliseconds) {
		QTextDocument* doc = this->document();
		if (doc == nullptr) {
			return;
		}
		qint32 firstVisible = 0;
		if (d->Parent != nullptr && d->Parent->d->Text != nullptr) {
			const QTextBlock visibleBlock = d->Parent->d->Text->getFirstVisibleBlock();
			if (visibleBlock.isValid()) {
				firstVisible = visibleBlock.blockNumber();
			}
		}
		const qint32 headFrom = qMax<qint32>(0, firstVisible - ViewportHighlightOverscan);
		const qint32 headTo = qMin<qint32>(doc->blockCount(), headFrom + ViewportHighlightCount);

		d->Parent->d->Rehighlighting = true;
		QTextBlock block = doc->findBlockByNumber(headFrom);
		while (block.isValid() && block.blockNumber() < headTo) {
			this->rehighlightBlock(block);
			block = block.next();
		}
		d->Parent->d->Rehighlighting = false;

		d->ProgressiveNext = 0;
		d->ProgressiveSkipFrom = headFrom;
		d->ProgressiveSkipTo = headTo;
		d->ProgressiveSliceMs = qMax<qint32>(1, sliceMilliseconds);
		d->stepProgressiveHighlight();
	}

	/*!
		\since YSS 0.18.0

		分片着色的单步：在时间预算内尽量多地着色，做完就发出 wholeDocumentHighlighted()。

		\note 分片期间如果用户改动了光标之前的文本，块号会错位，极少数块可能因此被跳过、
		保持装载时的无格式状态。被改动的行本身不受影响（QSyntaxHighlighter 对内容变化范围
		是同步重新着色的），而且分片总共只持续不到一秒，因此这里不做额外的重扫。
	*/
	void SyntaxHighlighterPrivate::stepProgressiveHighlight() {
		QTextDocument* doc = q->document();
		if (doc == nullptr || ProgressiveNext < 0) {
			return;
		}
		QElapsedTimer timer;
		timer.start();
		Parent->d->Rehighlighting = true;
		QTextBlock block = doc->findBlockByNumber(ProgressiveNext);
		while (block.isValid() && timer.elapsed() < ProgressiveSliceMs) {
			if (block.blockNumber() == ProgressiveSkipFrom) {
				block = doc->findBlockByNumber(ProgressiveSkipTo);
				continue;
			}
			q->rehighlightBlock(block);
			block = block.next();
		}
		Parent->d->Rehighlighting = false;
		if (block.isValid()) {
			ProgressiveNext = block.blockNumber();
			// 分片要持续几秒，期间按固定间隔刷新一次缩略图，避免它一直空着。
			ProgressiveElapsedMs += timer.elapsed();
			if (ProgressiveElapsedMs >= ProgressiveOverviewIntervalMs) {
				ProgressiveElapsedMs = 0;
				emit q->wholeDocumentHighlighted();
			}
			QTimer::singleShot(0, q, [this]() { stepProgressiveHighlight(); });
		}
		else {
			ProgressiveNext = -1;
			emit q->wholeDocumentHighlighted();
		}
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
		d->Parent->addMessage(currentBlock().blockNumber(),
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
		d->Parent->addMessage(currentBlock().blockNumber(),
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
		d->Parent->addMessage(currentBlock().blockNumber(),
			DocumentMessage(DocumentMessage::Info, message, currentBlock().blockNumber(), columnNumber, length, code, helpUrl, fixeAdvice));
	}

	/*!
		\since YSS 0.18.0
		\a hint 要创建的内联提示。

		为当前文本块创建一条内联提示，你可以在onBlockChanged()中调用这个函数，把不属于文档内容的辅助文字
		（例如函数调用的形参名）插进字符流里显示。提示会把后面的文字推开，但不会修改文档内容。

		与消息一样，每次重新着色一行时该行上已有的提示都会被自动清除，因此这里只需要给出当前这次解析的结果。
		位置小于1的提示无法推开后续文字，会被忽略。
	*/
	void SyntaxHighlighter::createInlayHint(const InlayHint& hint) {
		if (not d->AutoRenderInlayHint) {
			return;
		}
		if (hint.getText().isEmpty()) {
			return;
		}
		addInlayHint(currentBlock().blockNumber(), hint);
	}

	/*!
		\fn void SyntaxHighlighter::addInlayHint(qint32 blockNumber, const InlayHint& hint)
		\since YSS 0.18.0
		\a blockNumber 块号，从 0 开始。
		\a hint 要添加的内联提示。

		给指定文本块添加一条内联提示，提示随该文本块一起移动。
	*/
	void SyntaxHighlighter::addInlayHint(qint32 blockNumber, const InlayHint& hint) {
		if (blockNumber < 0) {
			return;
		}
		__Private__::InlayHintList* list = d->Parent->getBlockData<__Private__::InlayHintList>(blockNumber);
		if (list == nullptr) {
			list = new __Private__::InlayHintList();
			d->Parent->setBlockData<__Private__::InlayHintList>(blockNumber, list);
		}
		list->Hints.append(hint);
		emit inlayHintChangedForLine(blockNumber);
	}

	/*!
		\fn void SyntaxHighlighter::clearInlayHints(qint32 blockNumber)
		\since YSS 0.18.0
		\a blockNumber 块号，从 0 开始。

		清空指定文本块上的内联提示。着色每个文本块之前都会调用它，因此一个文本块上的提示总是当前这次解析的结果。
	*/
	void SyntaxHighlighter::clearInlayHints(qint32 blockNumber) {
		__Private__::InlayHintList* list = d->Parent->getBlockData<__Private__::InlayHintList>(blockNumber);
		if (list == nullptr or list->Hints.isEmpty()) {
			return;
		}
		list->Hints.clear();
		emit inlayHintChangedForLine(blockNumber);
	}

	/*!
		\fn QList<InlayHint> SyntaxHighlighter::getInlayHints(qint32 blockNumber) const
		\since YSS 0.18.0
		\a blockNumber 块号，从 0 开始。

		\return 该文本块上的内联提示。没有提示时返回空列表。
	*/
	QList<InlayHint> SyntaxHighlighter::getInlayHints(qint32 blockNumber) const {
		__Private__::InlayHintList* list = d->Parent->getBlockData<__Private__::InlayHintList>(blockNumber);
		if (list == nullptr) {
			return QList<InlayHint>();
		}
		return list->Hints;
	}

	/*!
		\fn QMap<qint32, QList<InlayHint>> SyntaxHighlighter::getAllInlayHints() const
		\since YSS 0.18.0
		\return 文档中所有需要显示的内联提示，以块号为键。

		只有确实带有提示的文本块才会出现在结果里。
	*/
	QMap<qint32, QList<InlayHint>> SyntaxHighlighter::getAllInlayHints() const {
		QMap<qint32, QList<InlayHint>> result;
		const QMap<qint32, __Private__::InlayHintList*> allData = d->Parent->getAllBlockData<__Private__::InlayHintList>();
		for (auto it = allData.begin(); it != allData.end(); ++it) {
			if (it.value()->Hints.isEmpty()) {
				continue;
			}
			result.insert(it.key(), it.value()->Hints);
		}
		return result;
	}

	/*!
		\since YSS 0.18.0
		\a positionInBlock 提示插入的位置（块内字符偏移，提示显示在该位置之前，必须大于0）。
		\a text 提示文字。
		\a color 提示文字颜色，默认为无效颜色，此时使用编辑器默认的弱化文字颜色。
		\a paddings 提示文字左右两侧的内边距（像素），默认为8。

		与 InlayHint 版本的 createInlayHint 相同，只是省略了构造 InlayHint 的步骤。
	*/
	void SyntaxHighlighter::createInlayHint(qint32 positionInBlock, const QString& text, const QColor& color, qint32 paddings) {
		createInlayHint(InlayHint(positionInBlock, text, color, paddings));
	}

	/*!
		\since YSS 0.18.0
		\a autoRender 是否自动创建并渲染内联提示，默认为true。

		设置为false时，createInlayHint不会再创建任何提示，已经创建的提示会被清除，文本流中为提示注入的空隙也会一并消失。
	*/
	void SyntaxHighlighter::setAutoRenderInlayHint(bool autoRender) {
		if (d->AutoRenderInlayHint == autoRender) {
			return;
		}
		d->AutoRenderInlayHint = autoRender;
		d->Parent->removeAllBlockData<__Private__::InlayHintList>();
		// 重新着色一遍以使注入的空隙随之出现或消失，rehighlight_s内部会屏蔽由此产生的文件改变信号。
		rehighlight_s();
	}

	/*!
		\since YSS 0.18.0
		\return 是否自动创建并渲染内联提示。
	*/
	bool SyntaxHighlighter::isAutoRenderInlayHint() const {
		return d->AutoRenderInlayHint;
	}
}