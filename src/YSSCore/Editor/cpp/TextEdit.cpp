#include <QtCore/qbytearray.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstringconverter.h>
#include <QtCore/qtimer.h>
#include <QtGui/qsyntaxhighlighter.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qplaintextedit.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <Utility/FileOperation.h>
#include "Editor/ColorThemeProvider.h"
#include "Editor/HoverInfoProvider.h"
#include "Editor/LangServer.h"
#include "Editor/LangServerManager.h"
#include "Editor/private/HoverInfoProvider_p.h"
#include "Editor/private/TabCompleterProvider_p.h"
#include "Editor/private/TextEdit_p.h"
#include "Editor/SyntaxHighlighter.h"
#include "Editor/TabCompleterProvider.h"
#include "Editor/TextEdit.h"
#include "General/YSSLogger.h"

namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::TextEdit
		\brief 这是YSS最关键的功能：代码编辑器.
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup LangService FileService

		TextEdit是Yayin Story Studio中最关键、最核心的功能，即代码编辑器。

		此类相比于Qt提供的QPlainTextEdit，额外提供了其所缺失的现代代码编辑必备的几项基本功能：
		\list
			\li 自动缩进
			\li 多行Tab的缩进和反缩进
			\li 行号
			\li 鼠标悬停提示、Tab补全
		\endlist

		基本上讲，以上这些额外补充的功能就已经满足了一个现代代码编辑器的基本需求。

		\section1 语言服务
		由于TextEdit是设计为Yayin Story Studio的内置代码编辑器，因此这个类的耦合性要稍微高一些。如果要
		提供语言语法支持，就不能单独使用这个类，它会通过访问YSSCore::Editor::LangServerManager
		来获取语言服务器的支持，并且在编辑器中提供语法高亮、鼠标悬停提示、Tab补全等功能。

		因此，如果要为TextEdit提供语言支持，您需要首先实现一个继承自YSSCore::Editor::LangServer的类，
		并且在其中实现语法高亮、鼠标悬停提示、Tab补全等功能。然后，您需要将这个语言服务器注册到YSSCore::Editor::LangServerManager中，
		TextEdit会自动按扩展名选择合适的语言服务器来提供支持。

		\section1 标准文件编辑框架
		为了统一Yayin Story Studio中全部的打开文件->路由合适的打开方式->编辑->保存的留存，该类
		继承自YSSCore::Editor::FileEditWidget。并在YSSCore::Editor::FileServerManager中作为默认的通用文件打开方式使用。

		\section1 内联提示
		内联提示是指在字符之间插入显示、但不属于文档内容的辅助文字，例如函数调用处的形参名提示。
		它由 YSSCore::Editor::SyntaxHighlighter::createInlayHint 在着色过程中创建，随该行一起刷新，
		可以像消息一样通过 YSSCore::Editor::SyntaxHighlighter::getInlayHints 取回，
		并通过 YSSCore::Editor::SyntaxHighlighter::inlayHintChangedForLine 信号通知界面刷新。

		内联提示的实现方式是给提示位置前一个字符注入字距，把后面的文字推开，再在让出的空隙里绘制提示，
		因此它不修改文档内容：不会被保存、不会进入撤销栈、也不会增加光标位置。这也意味着位置为0的提示
		（行首之前没有锚点字符）无法显示，会被忽略。

		提示的字号默认为正文的60%，可以用 YSSCore::Editor::TextEdit::setInlayHintFontScale 调整。
		提示的显示用一个开关统一控制：YSSCore::Editor::TextEdit::setInlayHintVisible。关闭它只是不给提示注入空隙、也不绘制，
		提示数据本身依然保留，与"根本不产出提示"的 YSSCore::Editor::SyntaxHighlighter::setAutoRenderInlayHint 是两回事。

		\section1 待抉择建议
		和上面的内联提示不同，建议不是为了长期显示语法信息，而是"提出一个改动并等用户决定"：
		用 YSSCore::Editor::TextEdit::showLineSuggestions 或 showBlockSuggestions 进入待抉择状态，
		编辑器在文本右侧弹出一个浮动小窗显示建议内容，多行建议还会在插入位置画一条通栏粗实线作为指示。

		小窗左上角显示当前建议的序号与总数，右上角是接受(√)和拒绝(×)按钮；键盘上 Tab 或 Enter 接受、
		Esc 拒绝、方向键在各条建议之间切换，输入其他字符或点击正文则视为放弃。接受时编辑器直接改文档
		并发信号通知调用方，放弃时只发信号。一次可以给出多条建议。

		这套机制完全在控件层实现：不修改文档（除了用户接受的那一刻）、不改变行高、也不进入撤销栈。

		\section1 自定义文档数据
		TextEdit允许把任意数据挂到某一行上，用来缓存这一行解析出来的信息。让数据类继承
		YSSCore::Editor::ICustomDocumentData，然后交给编辑器即可：

		\code
		MyLineData* data = new MyLineData();
		textEdit->setBlockData(3, data);

		MyLineData* got = textEdit->getBlockData<MyLineData>(3);
		\endcode

		这组带类型参数的重载会自己给出类型标识，返回值也已经是对应的指针类型，日常使用它们就够了。
		只有在类型要等到运行时才能确定时，才需要用带std::type_index参数的重载：

		\code
		textEdit->setBlockData(3, std::type_index(typeid(MyLineData)), data);
		\endcode

		同一行上可以同时挂载多种不同类型的数据，它们互不干扰；同一行同一种类型只会保存一个对象，重复设置会先释放旧对象。
		行号对应的行不存在、或者该行没挂这种类型的数据时，读取会返回空指针，调用方需要判空。

		数据交给编辑器之后，生命周期就由编辑器管理：行被删除、行被合并、或整个文档被替换时数据会被释放，
		因此不要手动删除数据对象，也不要把同一个对象挂到多行上。

		数据可以感知自己被移动到了别的行：当编辑器读取数据、发现行号与上次记录的不同时，会调用
		ICustomDocumentData::onBlockNumberChanged，派生类可以借此修正自身缓存的、与行号有关的信息。

		\warning 请注意，一旦决定使用自定义文档数据，就不要再使用QTextBlock::setUserData()，因为它们会互相干扰，导致数据丢失。

		\note 一般来说，TextEdit单独使用或组合使用，不应该继承此类，但考虑到潜在需求，因此也未设置final。

		\warning 这类在0.14.0版本调整过虚函数，ABI完全不兼容。

		\note 部分API不是在TextEdit.cpp中实现的：自定义文档数据的相关方法位于CustomDocumentData.cpp，消息相关的方法位于DocumentMessage.cpp。
	*/

	/*!
		\since YSS 0.13.0
		\a parent 父级窗口部件

		TextEdit的构造函数。
	*/
	TextEdit::TextEdit(QWidget* parent) :YSSCore::Editor::FileEditWidget(parent) {
		d = new YSSCore::__Private__::TextEditPrivate;
		d->q = this;
		// 新建的文档在 Windows 上按平台习惯用 CRLF；打开已有文件时会按文件本身的行尾覆盖。
#ifdef Q_OS_WIN
		d->UseCRLF = true;
#endif
		this->setMouseTracking(true);
		

		d->Layout = new QGridLayout(this);
		d->Layout->setSpacing(0);
		d->Layout->setContentsMargins(0, 0, 0, 0);

		d->Text = new YSSCore::__Private__::InlayHintTextEdit(this, this);
		d->Text->setLineWrapMode(QPlainTextEdit::NoWrap);
		d->Text->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->Layout->addWidget(d->Text, 0, 1);
		// 行号栏是自绘控件，依赖编辑区（文档布局与滚动位置），因此在 Text 之后再建。
		d->Line = new YSSCore::__Private__::LineNumberBar(d->Text, this);
		d->Layout->addWidget(d->Line, 0, 0);
		d->TabCompleterWidget = new YSSCore::__Private__::TabCompleterWidget(this);
		d->TabCompleterWidget->hide();
		d->HoverInfoWidget = new YSSCore::__Private__::HoverInfoWidget(d->Text);
		d->HoverInfoWidget->hide();
		// 建议浮窗是编辑区的子控件，位置直接用视口坐标，滚动时由 TextEditPrivate 跟着调整。
		d->SuggestionWidget = new YSSCore::__Private__::SuggestionWidget(this, d->Text->viewport());
		d->SuggestionWidget->hide();
		connect(d->SuggestionWidget, &YSSCore::__Private__::SuggestionWidget::accepted,
			this->d, &YSSCore::__Private__::TextEditPrivate::acceptSuggestion);
		connect(d->SuggestionWidget, &YSSCore::__Private__::SuggestionWidget::rejected,
			this->d, [this]() { this->d->dismissSuggestion(true); });
		// 临时测试入口：点一次弹出建议，再点收起，交替演示行内建议与多行建议。
		d->TestButton = new QPushButton(QStringLiteral("T"), d->Text->viewport());
		d->TestButton->setFocusPolicy(Qt::NoFocus);
		d->TestButton->setFixedSize(22, 20);
		d->TestButton->setToolTip(QStringLiteral("Test inlay suggestions"));
		connect(d->TestButton, &QPushButton::clicked, this, [this]() { d->runSuggestionDemo(); });
		d->positionTestButton();
		d->Text->installEventFilter(d);
		d->Text->viewport()->setMouseTracking(true);
		d->Text->viewport()->installEventFilter(d);
		d->Line->installEventFilter(d);

		setTextFont(QFont("Microsoft YaHei"));

		d->Overview = new YSSCore::__Private__::DocumentOverviewLabel(this, this);
		d->Layout->addWidget(d->Overview, 0, 2);

		// 底部横向滚动轴：整个控件底部统一横向滚动 Text 的内容
		d->HScrollBar = new QScrollBar(Qt::Horizontal, this);
		d->Layout->addWidget(d->HScrollBar, 1, 0, 1, 3);
		d->Layout->setRowStretch(0, 1);
		auto textHBar = d->Text->horizontalScrollBar();
		connect(textHBar, &QScrollBar::rangeChanged, this, [this, textHBar](int min, int max) {
			d->HScrollBar->setRange(min, max);
			d->HScrollBar->setPageStep(textHBar->pageStep());
			d->HScrollBar->setSingleStep(textHBar->singleStep());
			});
		connect(textHBar, &QScrollBar::valueChanged, d->HScrollBar, &QScrollBar::setValue);
		connect(d->HScrollBar, &QScrollBar::valueChanged, textHBar, &QScrollBar::setValue);

		d->HoverTimer = new QTimer(this);
		d->HoverTimer->setInterval(d->HoverTimeout);

		d->FindAndReplaceWidget = new YSSCore::__Private__::TextEditFindAndReplace(this);
		d->FindAndReplaceWidget->resize(340, 180);
		d->FindAndReplaceWidget->move(this->width() - 352, 0);
		d->FindAndReplaceWidget->hide();
		connect(d->Text->document(), &QTextDocument::blockCountChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onBlockCountChanged);
		connect(d->Text->document(), &QTextDocument::blockCountChanged, this, [this](qint32 count) {
			emit messageChanged();
			});
		// 行号栏只画可见行，内容或布局变了重绘一次即可，不需要维护行号文本。
		connect(d->Text->document(), &QTextDocument::contentsChanged, d->Line, [this]() { d->Line->update(); });
		connect(d->Text->document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged,
			d->Line, [this](const QSizeF&) { d->Line->update(); });
		connect(d->Text->verticalScrollBar(), &QScrollBar::valueChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onScrollBarChanged);
		connect(d->Text, &QPlainTextEdit::cursorPositionChanged, this->d, &YSSCore::__Private__::TextEditPrivate::onCursorPositionChanged);
		connect(d->HoverTimer, &QTimer::timeout, this->d, &YSSCore::__Private__::TextEditPrivate::onHoverTimeout);
		connect(d->Text, &QPlainTextEdit::textChanged, [this]() {
			// 整篇着色期间文档会短暂处于"还没有格式"的状态，此时不要去刷新缩略图，
			// 否则拿到的是空数据、缩略图会整片变白；着色结束后由 wholeDocumentHighlighted 刷新一次。
			if (not d->Rehighlighting) {
				this->setFileChanged();
			}
			});
		// 文档被其他途径改动（撤销、粘贴菜单等）时，指向旧文本的建议已经失效，直接收起。
		connect(d->Text->document(), &QTextDocument::contentsChanged, this, [this]() {
			if (d->ApplyingSuggestion) {
				return;
			}
			if (d->SgtKind != YSSCore::__Private__::SuggestionKind::None) {
				d->dismissSuggestion(false);
			}
			});
		this->installEventFilter(d);
	}

	/*!
		\since YSS 0.13.0
		TextEdit的析构函数。
	*/
	TextEdit::~TextEdit() {
		// d->Text must be delete before d :
		// If we do nothing with this object, d->Text will be automaticly deleted by
		// Qt's parent-child system. However, due to our YSSCore::Editor::SyntaxHighlighter
		// object is initialized with this object, but ownership belongs to QTextDocument in d->Text.
		// IF SyntaxHighlighter wants to access some of this object's members in its destructor,
		// it will cause a crash as our d pointer is already deleted.
		// So, Delete d->Text first to make sure SyntaxHighlighter is deleted before we delete d.
		d->HoverInfoWidget->setParent(d->Text);
		d->TabCompleterWidget->setParent(d->Text);
		delete d->Text;
		delete d;
	}

	/*!
		\since YSS 0.13.0
		\a area 设置鼠标悬停提示的显示区域。

		如果不设置或设置为nullptr，那么悬停区域永远不会超出TextEdit的范围。
		这可能在很多情况下会导致内容被遮挡，
		因此我们建议您将悬停区域设置为一个更大的区域，如您的组件的主窗口等。
	*/
	void TextEdit::setHoverArea(QWidget* area) {
		d->HoverArea = area;
		QWidget* target = area ? area : d->Text;
		d->HoverInfoWidget->setParent(target);
		d->TabCompleterWidget->setParent(target);
	}

	/*!
		\since YSS 0.13.0
		\a text 设置TextEdit中的文本内容。

		此函数会替换掉TextEdit中原有的全部文本内容。
	*/
	void TextEdit::setPlainText(const QString& text) {
		d->Text->setPlainText(text);
		this->setFileChanged();
		emit messageChanged();
	}

	/*!
		\since YSS 0.13.0
		return TextEdit中的文本内容。此函数会返回TextEdit中全部的文本内容。
	*/
	QString TextEdit::getPlainText() const {
		return d->Text->toPlainText();
	}

	/*!
		\since YSS 0.13.0
		\a lineNumber 将光标移动到指定行

		行号从1开始。
	*/
	void TextEdit::moveCursorToLine(int lineNumber) {
		if (lineNumber < 1 || lineNumber > d->Text->document()->blockCount()) {
			return;
		}
		QTextCursor cursor = d->Text->textCursor();
		cursor.movePosition(QTextCursor::Start);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
		d->Text->setTextCursor(cursor);
	}

	/*!
		\since YSS 0.13.0
		return 当前光标所在的行号。行号从0开始。

		\warning 在0.16.0之前，行号从1开始。为了保持与Qt的QTextBlock::blockNumber()函数一致，0.16.0版本将行号调整为从0开始。
	*/
	int TextEdit::getCurrentLineNumber() const {
		QTextCursor cursor = d->Text->textCursor();
		return cursor.block().blockNumber();
	}

	/*!
		\since YSS 0.13.0
		\a ms 设置鼠标悬停提示的超时时间。单位为毫秒。
	*/
	void TextEdit::setHoverTimeout(qint32 ms) {
		d->HoverTimeout = ms;
		d->HoverTimer->setInterval(ms);
	}

	/*!
		\since YSS 0.13.0
		return 鼠标悬停提示的超时时间。单位为毫秒。
	*/
	qint32 TextEdit::getHoverTimeout() const {
		return d->HoverTimeout;
	}

	/*!
		\since YSS 0.13.0
		\a reload 设置Tab键是否使用空格进行缩进。

		默认为false，即使用制表符进行缩进。
	*/
	void TextEdit::setTabReload(bool reload) {
		d->ReloadTab = reload;
	}

	/*!
		\since YSS 0.13.0
		return Tab键是否使用空格进行缩进。
	*/
	bool TextEdit::isTabReload() const {
		return d->ReloadTab;
	}

	/*!
		\since YSS 0.13.0
		return TextEdit中用于显示文本内容的QTextDocument对象。

		通过这个对象，您可以实现更底层的文本操作，如语法高亮、文本格式化等。
	*/
	QTextDocument* TextEdit::getDocument() const {
		return d->Text->document();
	}

	/*!
		\since YSS 0.15.2
		return TextEdit中的光标对象。
	*/
	QTextCursor TextEdit::getTextCursor() const {
		return d->Text->textCursor();
	}

	/*!
		\since YSS 0.14.0
		return 字体的字号。
	*/
	qint32 TextEdit::getFontSize() const {
		return d->Text->font().pointSize();
	}

	/*!
		\since YSS 0.14.0
		\a size 设置字体的字号。
	*/
	void TextEdit::setFontSize(qint32 size) {
		QFont font = d->Text->document()->defaultFont();
		font.setPointSize(size);
		d->Text->document()->setDefaultFont(font);
		d->Line->refreshMetrics();
		if (d->FontMetrics) {
			delete d->FontMetrics;
			d->FontMetrics = nullptr;
		}
		d->FontMetrics = new QFontMetricsF(font);
		double rawSpaceWidth = d->FontMetrics->size(Qt::TextSingleLine, " ").width();
		double tabStopDistance = d->TabWidth * rawSpaceWidth * this->devicePixelRatioF();
		d->Text->setTabStopDistance(qMax(20.0, tabStopDistance));
		// 内联提示的空隙宽度由字体决定，字号变化后需要按新字号重新为可见行注入。
		d->Text->refreshInlayHintSpacings();
	}

	/*!
		\since YSS 0.15.2
		\a font 设置字体。

		设置用于显示文本内容的字体。这个设置只影响文本编辑区域和行号的字体，不干涉鼠标悬停提示和Tab补全的字体设置。
	*/
	void TextEdit::setTextFont(const QFont& font) {
		auto fontSize = d->Text->document()->defaultFont().pointSize();
		if (d->FontMetrics) {
			delete d->FontMetrics;
			d->FontMetrics = nullptr;
		}
		
		auto target = font;
		target.setPointSize(fontSize);
		d->Text->document()->setDefaultFont(target);
		d->Line->refreshMetrics();
		d->FontMetrics = new QFontMetricsF(target);
		double rawSpaceWidth = d->FontMetrics->size(Qt::TextSingleLine, " ").width();
		double tabStopDistance = d->TabWidth * rawSpaceWidth * this->devicePixelRatioF();
		d->Text->setTabStopDistance(qMax(20.0, tabStopDistance));
		// 内联提示的空隙宽度由字体决定，字体变化后需要按新字体重新为可见行注入。
		d->Text->refreshInlayHintSpacings();
	}

	/*!
		\since YSS 0.15.2

		return 用于显示文本内容的字体。
	*/
	QFont TextEdit::getTextFont() const {
		return d->Text->font();
	}

	/*!
		\since YSS 0.16.0
		\a width 设置Tab键的宽度，单位为空格数。

		默认值为4，即一个Tab键等于4个空格的宽度。
	*/
	void TextEdit::setTabWidth(qint32 width) {
		d->TabWidth = width;
		double rawSpaceWidth = d->FontMetrics->size(Qt::TextSingleLine, " ").width();
		double tabStopDistance = d->TabWidth * rawSpaceWidth * this->devicePixelRatioF();
		d->Text->setTabStopDistance(qMax(20.0, tabStopDistance));
	}

	/*!
		\since YSS 0.16.0
		return Tab键的宽度，单位为空格数。

		默认值为4，即一个Tab键等于4个空格的宽度。
	*/
	qint32 TextEdit::getTabWidth() const {
		return d->TabWidth;
	}

	/*!
		\since YSS 0.18.0
		\a scale 内联提示字号相对正文字号的缩放比例，默认 0.6。取值范围会被钳制在 0.1 到 2.0 之间。

		提示的字号决定了它需要在文本流中占用的空隙宽度，因此改变这个比例会重新为带有提示的行注入空隙。
	*/
	void TextEdit::setInlayHintFontScale(qreal scale) {
		const qreal bounded = qBound(0.1, scale, 2.0);
		if (qFuzzyCompare(d->InlayHintFontScale, bounded)) {
			return;
		}
		d->InlayHintFontScale = bounded;
		d->Text->refreshInlayHintSpacings();
	}

	/*!
		\since YSS 0.18.0
		\return 内联提示字号相对正文字号的缩放比例。
	*/
	qreal TextEdit::getInlayHintFontScale() const {
		return d->InlayHintFontScale;
	}

	/*!
		\since YSS 0.18.0
		\a visible 是否显示内联提示，默认为true。

		这是一个纯显示开关：关闭时语法高亮器照常产出提示、提示数据也依然可以通过
		YSSCore::Editor::SyntaxHighlighter::getInlayHints 取回，只是不再在文本流中为提示注入空隙，也不再绘制提示文字。
		如果你要的是"干脆不产出提示"，请用 YSSCore::Editor::SyntaxHighlighter::setAutoRenderInlayHint。

		切换会重新为带有提示的行注入（或收回）空隙，被收回的行也会重新绘制，因此已经画在屏幕上的提示会立刻消失。
	*/
	void TextEdit::setInlayHintVisible(bool visible) {
		if (d->InlayHintVisible == visible) {
			return;
		}
		d->InlayHintVisible = visible;
		d->Text->refreshInlayHintSpacings();
	}

	/*!
		\since YSS 0.18.0
		\return 是否显示内联提示。
	*/
	bool TextEdit::isInlayHintVisible() const {
		return d->InlayHintVisible;
	}

	/*!
		\since YSS 0.18.0
		\a lineNumber 建议要作用的行号，从 0 开始。
		\a suggestions 建议列表，每条建议是一行文字。
		\a pureAdd 为真时建议是"纯追加"：文字被画在行尾，接受时追加到行尾而不是替换整行；为假时替换整行。

		在指定行进入待抉择状态：编辑器在这一行右侧弹出一个浮动小窗显示当前建议，
		并等待用户抉择——Tab 或 Enter 接受，Esc 拒绝，方向键在多条建议之间切换，
		输入其他字符或点击正文则视为放弃。

		接受时编辑器会把该行替换为当前建议（pureAdd 时改为追加到行尾），然后发出 modifySuggestionAccepted ；
		放弃时发出 modifySuggestionRejected 。一次可以给出多条建议，浮窗左上角会显示"当前序号/总数"。
	*/
	void TextEdit::showLineSuggestions(qint32 lineNumber, const QStringList& suggestions, bool pureAdd) {
		QList<QStringList> items;
		for (const QString& suggestion : suggestions) {
			items.append(QStringList{ suggestion });
		}
		d->showSuggestion(YSSCore::__Private__::SuggestionKind::Inline, lineNumber, items, pureAdd);
	}

	/*!
		\since YSS 0.18.0
		\a insertLine 插入位置所在的行号，从 0 开始；建议会插入到这一行之前。传入文档行数表示追加到末尾。
		\a suggestions 建议列表，每条建议是若干行文字。

		和 showLineSuggestions 一样进入待抉择状态，区别在于这条建议会增加行，因此编辑器会在插入位置
		画一条通栏粗实线作为指示（左端是一个 > 形状）。接受时这些行被插入到 \a insertLine 之前并发出
		ghostTextAccepted ，放弃时发出 ghostTextRejected 。
	*/
	void TextEdit::showBlockSuggestions(qint32 insertLine, const QList<QStringList>& suggestions) {
		d->showSuggestion(YSSCore::__Private__::SuggestionKind::Block, insertLine, suggestions);
	}

	/*!
		\since YSS 0.18.0
		\a from 要删除的第一行，从 0 开始。
		\a size 要删除的行数。

		在指定行区间进入行删除的待抉择状态：编辑器在要删除的行上叠一层半透明红底，
		右侧浮窗只显示"行删除建议"标题与接受/拒绝按钮（没有序号和正文，因为删除只有一种结果）。
		接受时删除这些行并发出 blockRemoveAccepted ，放弃时发出 blockRemoveRejected 。
	*/
	void TextEdit::showBlockRemoveSuggestions(qint32 from, qint32 size) {
		d->showRemoveSuggestion(from, size);
	}

	/*!
		\since YSS 0.18.0

		主动退出待抉择状态，不发出被拒绝的信号。用户自己放弃时编辑器会发信号，调用方自己收起时用这个。
	*/
	void TextEdit::dismissSuggestions() {
		d->dismissSuggestion(false);
	}

	/*!
		\since YSS 0.18.0
		\return 当前是否处于待抉择状态。
	*/
	bool TextEdit::hasActiveSuggestions() const {
		return d->SgtKind != YSSCore::__Private__::SuggestionKind::None;
	}

	/*!
		\since YSS 0.14.0
		\a source 目标字符串，可以是普通字符串，也可以是正则表达式，取决于 \a sourceAsRe 参数。
		\a sourceAsRe 当为真时， \a source 被视作正则表达式进行查找；当为假时， \a source 被视作普通字符串进行查找。
		\a options 查找选项，完全与QTextDocument::find()函数的flags参数相同。
		\a multiSelection 当为真时，TextEdit会为每个匹配项创建选区以将它们临时高亮。请注意，这个临时选区只是视觉效果，不改变当前光标位置。

		return 一个包含所有匹配结果的QTextCursor构成的列表。这些QTextCursor的特性完全由
	*/
	QList<QTextCursor> TextEdit::findAll(const QString& source, bool sourceAsRe, QTextDocument::FindFlags options, bool multiSelection) const {
		QList<QTextCursor> results;
		if (sourceAsRe) {
			QRegularExpression re(source);
			QTextCursor cursor(d->Text->document());
			cursor.setPosition(0);
			while (!cursor.isNull() && !cursor.atEnd()) {
				QTextCursor found = d->Text->document()->find(re, cursor, options);
				if (found.isNull() || found.position() == cursor.position()) {
					break;
				}
				results.append(found);
				cursor = found;
			}
		}
		else {
			QTextCursor cursor(d->Text->document());
			cursor.setPosition(0);
			while (!cursor.isNull() && !cursor.atEnd()) {
				QTextCursor found = d->Text->document()->find(source, cursor, options);
				if (found.isNull() || found.position() == cursor.position()) {
					break;
				}
				results.append(found);
				cursor = found;
			}
		}
		if (multiSelection) {
			d->createFindAllMultiSelection(results);
		}
		return results;
	}

	/*!
		\since YSS 0.14.0
		清除findAll函数创建的临时选区。临时选区只是视觉效果，不改变当前光标位置。
	*/
	void TextEdit::clearFindAllSelection() {
		d->clearFindAllMultiSelection();
	}

	/*!
		\since YSS 0.14.0
		\a source 目标字符串，可以是普通字符串，也可以是正则表达式，取决于 \a sourceAsRe 参数。
		\a sourceAsRe 当为真时， \a source 被视作正则表达式进行查找；当为假时， \a source 被视作普通字符串进行查找。
		\a from 查找的起始位置，默认为-1，表示从当前光标位置开始查找。
		\a options 查找选项，完全与QTextDocument::find()函数的flags参数相同。
		\a relocate 当为真时，如果找到匹配项，光标会被移动到这个匹配项的位置；当为假时，光标位置不变。

		return 一个QTextCursor，表示找到的匹配项的位置。如果没有找到匹配项，返回一个isNull()为真的QTextCursor。
	*/
	QTextCursor TextEdit::findNext(const QString& source, bool sourceAsRe, qint32 from, QTextDocument::FindFlags options, bool relocate) const {
		if (from == -1) {
			from = d->Text->textCursor().position();
		}
		QTextCursor result;
		if (sourceAsRe) {
			QRegularExpression re(source);
			result = d->Text->document()->find(re, from, options);
		}
		else {
			result = d->Text->document()->find(source, from, options);
		}
		if (relocate && !result.isNull()) {
			d->Text->setTextCursor(result);
		}
		return result;
	}

	/*!
		\since YSS 0.14.0
		\a source 目标字符串，可以是普通字符串，也可以是正则表达式，取决于 \a sourceAsRe 参数。
		\a sourceAsRe 当为真时， \a source 被视作正则表达式进行查找；当为假时， \a source 被视作普通字符串进行查找。
		\a newText 要替换为的新字符串。
		\a options 查找选项，完全与QTextDocument::find()函数的flags参数相同。

		return 替换掉的匹配项数量。
	*/
	qint32 TextEdit::replaceAll(const QString& source, const QString& newText, bool sourceAsRe, QTextDocument::FindFlags options) {
		QList<QTextCursor> matches = findAll(source, sourceAsRe, options);
		QTextCursor opeCusor = d->Text->textCursor();
		opeCusor.beginEditBlock();
		for (QTextCursor& cursor : matches) {
			opeCusor.setPosition(cursor.selectionStart());
			opeCusor.setPosition(cursor.selectionEnd(), QTextCursor::KeepAnchor);
			opeCusor.insertText(newText);
		}
		opeCusor.endEditBlock();
		return matches.size();
	}

	/*!
		\since YSS 0.14.0
		\a source 目标字符串，可以是普通字符串，也可以是正则表达式，取决于 \a sourceAsRe 参数。
		\a newText 要替换为的新字符串。
		\a sourceAsRe 当为真时， \a source 被视作正则表达式进行查找；当为假时， \a source 被视作普通字符串进行查找。
		\a from 查找的起始位置，默认为-1，表示从当前光标位置开始查找。
		\a options 查找选项，完全与QTextDocument::find()函数的flags参数相同。
		\a relocate 当为真时，如果找到匹配项，光标会被移动到这个匹配项的位置；当为假时，光标位置不变。

		return 是否成功替换。
	*/
	bool TextEdit::replaceNext(const QString& source, const QString& newText, bool sourceAsRe, qint32 from, QTextDocument::FindFlags options, bool relocate) {
		QTextCursor match = findNext(source, sourceAsRe, from, options);
		if (match.isNull()) {
			return false;
		}
		else {
			match.insertText(newText);
			if (relocate) {
				d->Text->setTextCursor(match);
			}
			return true;
		}
	}

	/*!
		\since YSS 0.14.0
		显示查找和替换页面。这个页面会在TextEdit的右上角显示，您也可以通过拖动来调整它的位置。
		如果当前有选中的文本，那么这个选中的文本会被自动填充到查找输入框中。
		请注意，它默认可用快捷键Ctrl+F来打开，您也可以通过调用这个函数来打开它。
	*/
	void TextEdit::showFindAndReplace() {
		d->showFindAndReplace();
	}

	/*!
		\since YSS 0.16.0
		\a level 补全提示详细等级

		设置补全提示的详细等级。置为None时，不显示任何补全提示。

		值得指出的是，为了插件行为的一致性，补全等级设置不影响相关函数的调用，
		也就是说，即使补全等级设置为None，相关的补全函数仍然会被调用，只是它们的结果不会被显示出来。
	*/
	void TextEdit::setCompleterLevel(TabCompleterItem::CompleterLevel level) {
		d->CompleterLevel = level;
	}

	/*!
		\since YSS 0.16.0
		return 当前补全提示的详细等级。
	*/
	TabCompleterItem::CompleterLevel TextEdit::getCompleterLevel() const {
		return d->CompleterLevel;
	}

	/*!
		\since YSS 0.16.0
		\a filter 类型过滤值

		设置补全提示的类型过滤，是YSSCore::Editor::TabCompleterItem::TypeFlag的逻辑或。
	*/
	void TextEdit::setCompleterTypeFilter(TabCompleterItem::ItemTypes filter) {
		bool equal = (d->CompleterTypeFilter == filter);
		d->CompleterTypeFilter = filter;
		if (not equal && d->TabCompleterWidget->isVisible()) {
			d->TabCompleterWidget->reApplyFilter();
		}
	}

	/*!
		\since YSS 0.16.0

		return 当前补全提示的类型过滤值，是YSSCore::Editor::TabCompleterItem::TypeFlag的逻辑或。
	*/
	TabCompleterItem::ItemTypes TextEdit::getCompleterTypeFilter() const {
		return d->CompleterTypeFilter;
	}

	/*!
		\since YSS 0.16.0
		\a readOnly 设置TextEdit是否为只读模式。
	*/
	void TextEdit::setReadOnly(bool readOnly) {
		d->Text->setReadOnly(readOnly);
	}

	/*!
		\since YSS 0.16.0
		return TextEdit是否为只读模式。
	*/
	bool TextEdit::isReadOnly() const {
		return d->Text->isReadOnly();
	}

	/*!
		\since YSS 0.16.0
		return TextEdit的语法高亮器。

		如果当前文件没有对应的语言服务器，返回nullptr。
	*/
	SyntaxHighlighter* TextEdit::getSyntaxHighlighter() const {
		return d->Highlighter;
	}

	/*!
		\since YSS 0.13.0
		\a line 行号，从0开始。
		\a column 列号，从0开始。

		按给定行号和列重定位光标。行号和列号均从0开始。
		这是对基类纯虚函数的实现，不应直接调用此函数。请使用cursorToPosition()函数。

		return 如果成功将光标移动到指定位置，返回true；如果行号或列号无效，无法移动光标，则返回false。
	*/
	bool TextEdit::onCursorToPosition(qint32 line, qint32 column) {
		//vgDebug << "Cursor to position: line" << line << "column" << column;
		if (line < 0 || line >= d->Text->document()->blockCount()) {
			return false;
		}
		QTextBlock block = d->Text->document()->findBlockByNumber(line);
		if (column < 0 || column > block.length()) {
			return false;
		}
		QTextCursor cursor(block);
		cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
		d->Text->setTextCursor(cursor);
		return true;
	}

	/*!
		\since YSS 0.13.0
		\a path 文件路径

		打开一个文件。

		return 文件完全打开成功才会返回true，其他任何失败情况均返回false。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用openFile()函数。
	*/
	// 用"解码再编码是否还原"判断字节流是不是合法 UTF-8：非法字节会被解码器替换成 U+FFFD，
	// 再编码回去就对不上了；合法的 UTF-8（包括本身就含有 U+FFFD 的文本）可以精确还原。
	static bool isStrictUtf8(const QByteArray& bytes) {
		return QString::fromUtf8(bytes).toUtf8() == bytes;
	}

	bool TextEdit::onOpen(const YSSCore::General::YSSURI& uri, const QByteArray& data) {
		const QString ext = uri.getFileExt();
		const QByteArray bytes = data;
		
		QString text;
		if (bytes.startsWith(QByteArray::fromHex("efbbbf"))) {
			d->Encoding = YSSCore::__Private__::TextEditPrivate::TextEncoding::Utf8Bom;
			text = QString::fromUtf8(bytes.mid(3));
		}
		else if (bytes.startsWith(QByteArray::fromHex("fffe"))) {
			d->Encoding = YSSCore::__Private__::TextEditPrivate::TextEncoding::Utf16LE;
			text = QStringDecoder(QStringConverter::Utf16LE)(bytes.mid(2));
		}
		else if (bytes.startsWith(QByteArray::fromHex("feff"))) {
			d->Encoding = YSSCore::__Private__::TextEditPrivate::TextEncoding::Utf16BE;
			text = QStringDecoder(QStringConverter::Utf16BE)(bytes.mid(2));
		}
		else if (isStrictUtf8(bytes)) {
			d->Encoding = YSSCore::__Private__::TextEditPrivate::TextEncoding::Utf8;
			text = QString::fromUtf8(bytes);
		}
		else {
			d->Encoding = YSSCore::__Private__::TextEditPrivate::TextEncoding::Unsupported;
			text = QString::fromUtf8(bytes);
			vgWarningF << "File" << uri.toString() << "is not UTF-8/UTF-16 encoded, it is opened for display only and cannot be saved in place.";
		}
		d->UseCRLF = text.contains("\r\n");
		if (d->LangServer) {
			auto themeProvider = d->LangServer->getColorThemeProvider();
			disconnect(themeProvider, &YSSCore::Editor::ColorThemeProvider::themeModified,
				this, nullptr); // disconnect all slots connected to this signal
			disconnect(themeProvider, &YSSCore::Editor::ColorThemeProvider::currentThemeChanged,
				this, nullptr); // disconnect all slots connected to this signal
			d->LangServer = nullptr;
		}
		d->LangServer = YSSLSM->routeExt(ext);
		if (d->HoverInfoProvider) {
			d->HoverInfoProvider->setParent(nullptr);
			d->HoverInfoProvider->deleteLater();
			d->HoverInfoProvider = nullptr;
		}
		if (d->TabCompleter) {
			d->TabCompleter->setParent(nullptr);
			d->TabCompleter->deleteLater();
			d->TabCompleter = nullptr;
		}
		if (d->Highlighter) {
			d->Highlighter->setDocument(nullptr);
			d->Highlighter->setParent(nullptr);
			d->Highlighter->deleteLater();
			d->Highlighter = nullptr;
		}
		d->TabCompleterWidget->hide();
		d->HoverInfoWidget->hide();
		if (d->LangServer) {
			d->Highlighter = d->LangServer->createHighlighter(this);
			if (d->Highlighter) {
				connect(d->Highlighter, &YSSCore::Editor::SyntaxHighlighter::inlayHintChangedForLine,
					d->Text, &YSSCore::__Private__::InlayHintTextEdit::onInlayHintChangedForLine);
				// 缩略图的颜色来自各块布局里的格式，只有整篇着色结束后才是完整的。
				connect(d->Highlighter, &YSSCore::Editor::SyntaxHighlighter::wholeDocumentHighlighted,
					this, [this]() {
						if (d->Overview != nullptr) {
							d->Overview->recalculateAll();
						}
					});
				auto themeProvider = d->LangServer->getColorThemeProvider();
				d->Highlighter->onThemeChanged(themeProvider->getCurrentThemeStyleData());
				connect(themeProvider, &YSSCore::Editor::ColorThemeProvider::themeModified,
					this, [this, themeProvider](const QString& themeName) {
						if (d->Highlighter) {
							d->Highlighter->onThemeChanged(themeProvider->getCurrentThemeStyleData());
							d->Highlighter->rehighlight_s();
						}
					});
				connect(themeProvider, &YSSCore::Editor::ColorThemeProvider::currentThemeChanged,
					this, [this, themeProvider](const QString& themeName) {
						if (d->Highlighter) {
							d->Highlighter->onThemeChanged(themeProvider->getCurrentThemeStyleData());
							d->Highlighter->rehighlight_s();
						}
					});
			}
			d->TabCompleter = d->LangServer->createTabCompleter(this);
			d->HoverInfoProvider = d->LangServer->createHoverInfoProvider(this);
		}
		else {
			yInfoF << "No Language server found for extension:" << ext;
		}
		//highlighter 必须在 setText 之前挂上：QSyntaxHighlighter::setDocument 对非空文档会
		//排一个延迟的整篇重着色，那既等于把整篇再算一遍，又会在 cancelFileChanged 之后
		//从 QTextCursor 的编辑块里发出 textChanged，让新打开的文件错误地显示为已更改。
		//
		//挂上之后先把着色挂起：setPlainText 会对内容范围内的每一块同步回调一次，
		//而这几万次回调既看不到、也没必要，整篇着色统一交给 rehighlightProgressive_s()，
		//它会先把视口附近同步着色好（首帧就是带色的），其余分片补完。
		if (d->Highlighter != nullptr) {
			d->Highlighter->setHighlightSuspended(true);
		}
		d->Text->setPlainText(text);
		if (d->Highlighter != nullptr) {
			d->Highlighter->setHighlightSuspended(false);
			d->Highlighter->rehighlightProgressive_s();
		}
		cancelFileChanged();
		emit messageChanged();
		
		return true;
	}

	/*!
		\since YSS 0.13.0
		\a path 文件路径

		保存当前文件。

		return 成功时返回要写出的完整内容；失败时返回 \c{std::unexpected(true)}，
		调用方据此知道这是一次需要让用户知道的失败。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用saveFile()函数。
	*/
	std::expected<QByteArray, bool> TextEdit::onSave(const YSSCore::General::YSSURI& uri) {
		using TextEncoding = YSSCore::__Private__::TextEditPrivate::TextEncoding;
		const QString path = uri.getLocalPath();
		QFileInfo fileInfo(path);
		if (uri.isLocal() && fileInfo.exists() && not fileInfo.isWritable()) {
			if (not isSaveSilent()) {
				QMessageBox::warning(this,
					VITRL("YSS::editor.textEdit.readOnly.title"), VITRL("YSS::editor.textEdit.readOnly.message").arg(fileInfo.fileName()),
					QMessageBox::Ok);
			}
			vgWarning << "File is read-only:" << path << ",";
			return std::unexpected(true);
		}
		// 非 UTF-8/UTF-16 的文件只能另存，不能覆盖原件。
		const bool saveToOtherPath = uri != getUri();
		if (d->Encoding == TextEncoding::Unsupported and not saveToOtherPath) {
			if (not isSaveSilent()) {
				QMessageBox::warning(this,
					VITRL("YSS::editor.textEdit.unsupportedEncoding.title"),
					VITRL("YSS::editor.textEdit.unsupportedEncoding.message").arg(fileInfo.fileName()),
					QMessageBox::Ok);
			}
			vgWarning << "Refuse to overwrite file with unsupported encoding:" << path;
			return std::unexpected(true);
		}
		QString text = d->Text->toPlainText();
		if (d->UseCRLF) {
			text.replace("\n", "\r\n");
		}
		QByteArray data;
		switch (d->Encoding) {
		case TextEncoding::Utf8:
			data = text.toUtf8();
			break;
		case TextEncoding::Utf8Bom:
			data = QByteArray::fromHex("efbbbf") + text.toUtf8();
			break;
		case TextEncoding::Utf16LE:
			data = QByteArray::fromHex("fffe") + QStringEncoder(QStringConverter::Utf16LE)(text);
			break;
		case TextEncoding::Utf16BE:
			data = QByteArray::fromHex("feff") + QStringEncoder(QStringConverter::Utf16BE)(text);
			break;
		default:
			data = text.toUtf8();
			break;
		}

		if (d->Encoding == TextEncoding::Unsupported) {
			// 另存为 UTF-8 之后，这个编辑器就按 UTF-8 工作了。
			d->Encoding = TextEncoding::Utf8;
		}
		return data;
	}

	/*!
		\since YSS 0.13.0
		重新加载当前文件。

		return 当文件成功重新加载时才会返回true，其他任何失败情况均返回false。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用reloadFile()函数。
	*/
	bool TextEdit::onReload(const QByteArray& data) {
		if (isFileChanged()) {
			QMessageBox msgBox;
			msgBox.setWindowTitle(VITRL("YSS::editor.reloadWarning.title"));
			msgBox.setText(VITRL("YSS::editor.reloadWarning.message").arg(getFileName()));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Yes);
			int ret = msgBox.exec();
			switch (ret) {
			case QMessageBox::Yes:
				return onOpen(getUri(), data);
				break;
			case QMessageBox::Cancel:
				break;
			default:
				break;
			}
			return false;
		}
		else {
			return openFile(getUri(), data);
		}
	}

	/*!
		\since YSS 0.13.0
		复制选中的文本。

		return 如果成功复制文本，返回true；否则返回false。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用copy()函数。
	*/
	bool TextEdit::onCopy() {
		d->Text->copy();
		return true;
	}

	/*!
		\since YSS 0.13.0
		剪切选中的文本。

		return 如果成功剪切文本，返回true；否则返回false。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用cut()函数。
	*/
	bool TextEdit::onCut() {
		d->Text->cut();
		return true;
	}

	/*!
		\since YSS 0.13.0
		粘贴剪贴板中的文本。

		return 如果成功粘贴文本，返回true；否则返回false。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用paste()函数。
	*/
	bool TextEdit::onPaste() {
		d->Text->paste();
		return true;
	}

	/*!
		\since YSS 0.13.0
		撤销上一步操作。

		return 如果成功撤销操作，返回true；否则返回false。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用undo()函数。
	*/
	bool TextEdit::onUndo() {
		d->Text->undo();
		return true;
	}

	/*!
		\since YSS 0.13.0
		重做上一步被撤销的操作。

		return 如果成功重做操作，返回true；否则返回false。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用redo()函数。
	*/
	bool TextEdit::onRedo() {
		d->Text->redo();
		return true;
	}

	/*!
		\since YSS 0.13.0
		选择全部文本。

		return 如果成功选择全部文本，返回true；否则返回false。

		这是对基类纯虚函数的实现，不应直接调用此函数。请使用selectAll()函数。
	*/
	bool TextEdit::onSelectAll() {
		d->Text->selectAll();
		return true;
	}
}