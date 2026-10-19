#include <QtCore/qtimer.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpalette.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextlayout.h>
#include <QtGui/qtextobject.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qstyleoption.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <Widgets/LiquidGlassEffect.h>
#include <Widgets/ThemeManager.h>
#include "Editor/DocumentMessage.h"
#include "Editor/InlayHint.h"
#include "Editor/SyntaxHighlighter.h"
#include "Editor/private/InlayHint_p.h"
#include "Editor/private/TextEdit_p.h"
#include "Editor/TextEdit.h"

namespace YSSCore::__Private__ {
	TextEditFindAndReplace::TextEditFindAndReplace(YSSCore::Editor::TextEdit* parent) : Visindigo::Widgets::BorderFrame(parent) {
		this->parent = parent;
		auto font = this->font();
		font.setPointSizeF(font.pointSizeF() * 0.85);
		this->setFont(font);
		layout = new QGridLayout(this);
		titleLabel = new QLabel(VITRL("YSS::editor.far.title"), this);
		closeButton = new QPushButton(VITRL("Visindigo::general.close"), this);
		sourceAsReCheckBox = new QCheckBox(VITRL("YSS::editor.far.useRegExp"), this);
		caseSensitiveCheckBox = new QCheckBox(VITRL("YSS::editor.far.matchCase"), this);
		wholeWordCheckBox = new QCheckBox(VITRL("YSS::editor.far.wholeWord"), this);

		rawInputLabel = new QLabel(VITRL("YSS::editor.far.find"), this);
		rawInput = new QLineEdit(this);
		replaceInputLabel = new QLabel(VITRL("YSS::editor.far.replace"), this);
		replaceInput = new QLineEdit(this);

		matchCountLabel = new QLabel(this);
		findNextButton = new QPushButton(VITRL("YSS::editor.far.findNext"), this);
		replaceNextButton = new QPushButton(VITRL("YSS::editor.far.replaceNext"), this);
		replaceAllButton = new QPushButton(VITRL("YSS::editor.far.replaceAll"), this);

		layout->addWidget(titleLabel, 0, 0, 1, 3);
		layout->addWidget(closeButton, 0, 3);
		layout->addWidget(sourceAsReCheckBox, 1, 1);
		layout->addWidget(caseSensitiveCheckBox, 1, 2);
		layout->addWidget(wholeWordCheckBox, 1, 3);
		layout->addWidget(rawInputLabel, 2, 0);
		layout->addWidget(rawInput, 2, 1, 1, 3);
		layout->addWidget(replaceInputLabel, 3, 0);
		layout->addWidget(replaceInput, 3, 1, 1, 3);
		layout->addWidget(matchCountLabel, 4, 0);
		layout->addWidget(findNextButton, 4, 1);
		layout->addWidget(replaceNextButton, 4, 2);
		layout->addWidget(replaceAllButton, 4, 3);

		connect(closeButton, &QPushButton::clicked, this, [this]() {
			this->parent->clearFindAllSelection();
			this->hide();
			});

		connect(caseSensitiveCheckBox, &QCheckBox::checkStateChanged, this, [this]() {
			this->findAll();
			});

		connect(wholeWordCheckBox, &QCheckBox::checkStateChanged, this, [this]() {
			this->findAll();
			});

		connect(sourceAsReCheckBox, &QCheckBox::checkStateChanged, this, [this]() {
			this->findAll();
			});

		connect(rawInput, &QLineEdit::returnPressed, this, [this]() {
			this->findAll();
			});
		connect(findNextButton, &QPushButton::clicked, this, [this]() {
			auto rtn = this->parent->findNext(rawInput->text(), sourceAsReCheckBox->isChecked(), -1,
				(caseSensitiveCheckBox->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags()) |
				(wholeWordCheckBox->isChecked() ? QTextDocument::FindWholeWords : QTextDocument::FindFlags()), true);
			if (rtn.isNull()) {
				QMessageBox::information(this, VITRL("YSS::editor.far.notFound"), VITRL("YSS::editor.far.notFoundDesc").arg(rawInput->text()));
			}
			});

		connect(replaceNextButton, &QPushButton::clicked, this, [this]() {
			bool rtn = this->parent->replaceNext(rawInput->text(), replaceInput->text(), sourceAsReCheckBox->isChecked(), -1,
				(caseSensitiveCheckBox->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags()) |
				(wholeWordCheckBox->isChecked() ? QTextDocument::FindWholeWords : QTextDocument::FindFlags()), true);
			if (not rtn) {
				QMessageBox::information(this, VITRL("YSS::editor.far.notFound"), VITRL("YSS::editor.far.notFoundDesc").arg(rawInput->text()));
			}
			});

		connect(replaceAllButton, &QPushButton::clicked, this, [this]() {
			qint32 rtn = this->parent->replaceAll(rawInput->text(), replaceInput->text(), sourceAsReCheckBox->isChecked(),
				(caseSensitiveCheckBox->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags()) |
				(wholeWordCheckBox->isChecked() ? QTextDocument::FindWholeWords : QTextDocument::FindFlags()));
			QMessageBox::information(this, VITRL("YSS::editor.far.replaced"), VITRL("YSS::editor.far.replacedDesc").arg(rtn));
			});
	}

	TextEditFindAndReplace::~TextEditFindAndReplace() {}

	void TextEditFindAndReplace::findAll() {
		auto cursors = this->parent->findAll(rawInput->text(), sourceAsReCheckBox->isChecked(),
			(caseSensitiveCheckBox->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags()) |
			(wholeWordCheckBox->isChecked() ? QTextDocument::FindWholeWords : QTextDocument::FindFlags()), true);
		matchCountLabel->setText(VITRL("YSS::editor.far.matchCount").arg(cursors.size()));
	}

	void TextEditFindAndReplace::setFindText(const QString& text) {
		rawInput->setText(text);
		findAll();
	}

	DocumentOverviewLabel::DocumentOverviewLabel(YSSCore::Editor::TextEdit* editor, QWidget* parent) : QWidget(parent) {
		this->setFixedWidth(80);
		this->editor = editor;
		this->document = editor->getDocument();
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

		connect(this->document, &QTextDocument::contentsChange, this, &DocumentOverviewLabel::onContentsChange);
		connect(editor, &YSSCore::Editor::TextEdit::messageChanged,
			this, &DocumentOverviewLabel::onMessageChanged);
		connect(editor, &YSSCore::Editor::TextEdit::messageChangedForLine,
			this, &DocumentOverviewLabel::onMessageChangedForLine);

		if (this->document->blockCount() > 0) {
			recalculateAll();
		}
		rebuildErrorLines();

		// 视口指示器（QStyle绘制：Window颜色50%透明背景 + 白色边框）
		this->viewportIndicator = new ViewportIndicator(this);
		this->viewportIndicator->installEventFilter(this);

		auto* scrollBar = this->editor->d->Text->verticalScrollBar();
		connect(scrollBar, &QScrollBar::valueChanged, this, &DocumentOverviewLabel::updateViewportIndicator);
		connect(scrollBar, &QScrollBar::rangeChanged, this, &DocumentOverviewLabel::updateViewportIndicator);
		updateViewportIndicator();
	}

	void DocumentOverviewLabel::resizeEvent(QResizeEvent* event) {
		QWidget::resizeEvent(event);
		// To solve the problem of incorrect size when the window maximizes.
		QTimer::singleShot(0, this, [this]() {
			recalculateAll();
			updateViewportIndicator();
			});
	}

	QSize DocumentOverviewLabel::sizeHint() const {
		return QSize(80, 0);
	}

	void DocumentOverviewLabel::paintEvent(QPaintEvent* event) {
		QWidget::paintEvent(event);

		QPainter painter(this);
		const int lineCount = this->lineColors.size();
		if (lineCount == 0) return;

		const int h = this->height();

		bool lessHalf = (lineCount * 3 < h);
		for (int y = 0; y < h; ++y) {
			int i = y * lineCount / h;
			if (i >= lineCount) break;

			// error line indicator
			if (this->errorLines.contains(i)) {
				QPen errorPen(Qt::red, 4);
				painter.setPen(errorPen);

				int lineTop, lineBottom;
				if (y <= 1) {
					lineTop = y;  lineBottom = y + 4;
				} else if (y >= h - 2) {
					lineTop = y - 4;  lineBottom = y;
				} else {
					lineTop = y - 2;  lineBottom = y + 2;
				}
				painter.drawLine(3, lineTop, 3, lineBottom);
			}

			if (lessHalf) {
				int i_n = (qMin(y + 1, h - 1)) * lineCount / h;
				if (i_n == i) continue;
			}
			const QList<QColor>& colors = this->lineColors[i];

			for (int x = 0; x < colors.size(); ++x) {
				painter.setPen(colors[x]);
				painter.drawPoint(x + 10, y);
			}
		}
	}

	void DocumentOverviewLabel::onContentsChange(int position, int charsRemoved, int charsAdded) {
		if (this->document == nullptr) return;

		const int blockCount = this->document->blockCount();

		while (this->lineColors.size() < blockCount) {
			this->lineColors.append(QList<QColor>());
		}
		while (this->lineColors.size() > blockCount) {
			this->lineColors.removeLast();
		}

		QTextBlock startBlock = this->document->findBlock(position);
		int startBlockNum = startBlock.blockNumber();

		int endBlockNum = startBlockNum;
		int endPos = position + qMax(charsRemoved, charsAdded);
		if (endPos > 0) {
			QTextBlock endBlock = this->document->findBlock(endPos - 1);
			endBlockNum = endBlock.blockNumber();
		}

		for (int i = startBlockNum; i <= endBlockNum && i < blockCount; ++i) {
			recalculateBlock(i);
		}

		this->update();
		updateViewportIndicator();
	}

	void DocumentOverviewLabel::recalculateAll() {
		if (this->document == nullptr) return;

		const int blockCount = this->document->blockCount();
		this->lineColors.clear();
		this->lineColors.reserve(blockCount);

		for (int i = 0; i < blockCount; ++i) {
			this->lineColors.append(QList<QColor>());
			recalculateBlock(i);
		}

		this->update();
	}

	void DocumentOverviewLabel::recalculateBlock(int blockNumber) {
		if (this->document == nullptr) return;

		QTextBlock block = this->document->findBlockByNumber(blockNumber);
		if (!block.isValid()) return;

		const int width = this->width() - 10;
		const QString text = block.text();
		const int len = qMin(text.length(), width);

		const QColor defaultColor = this->palette().color(QPalette::Text);
		QList<QColor> colors(len, defaultColor);

		QTextLayout* layout = block.layout();
		if (layout != nullptr) {
			const auto& formats = layout->formats();
			for (const auto& range : formats) {
				const QBrush fg = range.format.foreground();
				if (fg.style() == Qt::NoBrush || !fg.color().isValid()) continue;

				const int start = qMax(range.start, 0);
				const int end = qMin(range.start + range.length, len);
				for (int x = start; x < end; ++x) {
					colors[x] = fg.color();
				}
			}
		}

		if (blockNumber < this->lineColors.size()) {
			this->lineColors[blockNumber] = std::move(colors);
		}
	}

	void DocumentOverviewLabel::onMessageChanged() {
		rebuildErrorLines();
	}

	void DocumentOverviewLabel::onMessageChangedForLine(qint32 lineNumber) {
		if (!this->editor) return;

		if (this->editor->hasMessage(lineNumber)) {
			if (!this->errorLines.contains(lineNumber)) {
				this->errorLines.append(lineNumber);
			}
		} else {
			this->errorLines.removeAll(lineNumber);
		}
		this->update();
	}

	void DocumentOverviewLabel::rebuildErrorLines() {
		this->errorLines.clear();
		if (this->editor == nullptr) return;

		auto allMessages = this->editor->getAllMessages();
		for (auto it = allMessages.begin(); it != allMessages.end(); ++it) {
			if (!it.value().isEmpty()) {
				this->errorLines.append(it.key());
			}
		}
		this->update();
	}

	void DocumentOverviewLabel::updateViewportIndicator() {
		if (!this->editor || !this->viewportIndicator) return;

		auto* scrollBar = this->editor->d->Text->verticalScrollBar();
		const int maximum = scrollBar->maximum();
		const int pageStep = scrollBar->pageStep();
		const int totalRange = maximum + pageStep;
		if (totalRange <= 0) {
			this->viewportIndicator->hide();
			return;
		}

		const int h = this->height();
		const int indicatorH = qMax(h * pageStep / totalRange, 5);
		const int indicatorY = maximum > 0
			? (h - indicatorH) * scrollBar->value() / maximum
			: 0;

		this->viewportIndicator->setGeometry(0, indicatorY, this->width(), indicatorH);
		this->viewportIndicator->show();
	}

	bool DocumentOverviewLabel::eventFilter(QObject* obj, QEvent* event) {
		if (obj == this->viewportIndicator) {
			if (event->type() == QEvent::MouseButtonPress) {
				auto* me = static_cast<QMouseEvent*>(event);
				if (me->button() == Qt::LeftButton) {
					this->draggingViewport = true;
					this->dragStartY = me->globalPosition().y();
					this->dragStartScrollValue = this->editor->d->Text->verticalScrollBar()->value();
					return true;
				}
			} else if (event->type() == QEvent::MouseMove) {
				if (this->draggingViewport) {
					auto* me = static_cast<QMouseEvent*>(event);
					int deltaY = me->globalPosition().y() - this->dragStartY;
					auto* scrollBar = this->editor->d->Text->verticalScrollBar();
					const int maximum = scrollBar->maximum();
					const int pageStep = scrollBar->pageStep();
					const int totalRange = maximum + pageStep;
					const int h = this->height();
					const int indicatorH = qMax(h * pageStep / totalRange, 5);

					if (h - indicatorH > 0 && maximum > 0) {
						int newValue = this->dragStartScrollValue + deltaY * maximum / (h - indicatorH);
						newValue = qBound(scrollBar->minimum(), newValue, maximum);
						scrollBar->setValue(newValue);
					}
					return true;
				}
			} else if (event->type() == QEvent::MouseButtonRelease) {
				this->draggingViewport = false;
				return true;
			}
		}
		return QWidget::eventFilter(obj, event);
	}

	void ViewportIndicator::paintEvent(QPaintEvent*) {
		QStyleOption opt;
		opt.initFrom(this);
		QPainter p(this);

		QColor bg = opt.palette.color(QPalette::Window);
		bg.setAlpha(128);
		p.fillRect(rect(), bg);

		p.setPen(QPen(Qt::white, 2));
		p.drawRect(rect().adjusted(1, 1, -2, -2));
	}

	InlayHintTextEdit::InlayHintTextEdit(YSSCore::Editor::TextEdit* editor, QWidget* parent) : QTextEdit(parent) {
		this->editor = editor;
	}

	/*!
		绘制文本本身之后，再把内联提示画到文本流为它让出的空隙里。

		提示的空隙由 YSSCore::Editor::SyntaxHighlighter 通过字距注入产生，那个字距只存在于布局层，
		并不在文档里，所以文档内容、保存结果和撤销栈都不受影响，提示的文字需要由这里自己画上去。
	*/
	void InlayHintTextEdit::paintEvent(QPaintEvent* event) {
		QTextEdit::paintEvent(event);
		if (this->editor == nullptr) {
			return;
		}
		QPainter painter(this->viewport());
		painter.setClipRect(event->rect());
		painter.setRenderHint(QPainter::Antialiasing);
		this->paintInlayHints(painter, event->rect());
		this->paintRemoveSuggestion(painter, event->rect());
		this->paintSuggestionIndicator(painter, event->rect());
		this->paintPureAddSuggestion(painter, event->rect());
	}

	/*!
		\a lineNumber 提示发生变化的行号。

		只重绘这一行，避免每次着色都重绘整个视口。
	*/
	void InlayHintTextEdit::onInlayHintChangedForLine(qint32 lineNumber) {
		QTextDocument* document = this->document();
		if (document == nullptr) {
			return;
		}
		const QTextBlock block = document->findBlockByNumber(lineNumber);
		if (not block.isValid()) {
			this->viewport()->update();
			return;
		}
		const QRect lineRect = this->cursorRect(QTextCursor(block));
		if (not lineRect.isValid()) {
			this->viewport()->update();
			return;
		}
		this->viewport()->update(QRect(0, lineRect.top() - 2, this->viewport()->width(), lineRect.height() + 4));
	}

	/*!
		提示的空隙宽度取决于字体，字号或字体变化后必须重新注入一遍字距，否则空隙会与提示文字的大小对不上；
		显示开关切换时同样需要重新注入（或收回）空隙，因此也走这里。
		这里只为确实带有提示的行重新着色，而不是整篇重新着色：整篇重新着色在打开大文件时太昂贵了。
	*/
	void InlayHintTextEdit::refreshInlayHintSpacings() {
		if (this->editor == nullptr) {
			return;
		}
		YSSCore::Editor::SyntaxHighlighter* highlighter = this->editor->getSyntaxHighlighter();
		QTextDocument* document = this->document();
		if (highlighter == nullptr or document == nullptr) {
			return;
		}
		const QList<qint32> lines = highlighter->getAllInlayHints().keys();
		if (lines.isEmpty()) {
			return;
		}
		const bool previous = this->editor->d->Rehighlighting;
		this->editor->d->Rehighlighting = true;
		for (qint32 line : lines) {
			highlighter->rehighlightBlock(document->findBlockByNumber(line));
		}
		this->editor->d->Rehighlighting = previous;
	}

	void InlayHintTextEdit::paintInlayHints(QPainter& painter, const QRect& clipRect) {
		QTextDocument* document = this->document();
		if (document == nullptr) {
			return;
		}
		YSSCore::Editor::SyntaxHighlighter* highlighter = this->editor->getSyntaxHighlighter();
		if (highlighter == nullptr or not this->editor->isInlayHintVisible()) {
			return;
		}
		qint32 firstLine = 0;
		qint32 lastLine = 0;
		if (not this->visibleLines(firstLine, lastLine, clipRect)) {
			return;
		}
		const QFont hintFont = YSSCore::__Private__::inlayHintFont(document->defaultFont(),
			this->editor->getInlayHintFontScale());
		painter.setFont(hintFont);
		for (qint32 line = firstLine; line <= lastLine; ++line) {
			const QList<YSSCore::Editor::InlayHint> hints = highlighter->getInlayHints(line);
			if (hints.isEmpty()) {
				continue;
			}
			const QTextBlock block = document->findBlockByNumber(line);
			for (const YSSCore::Editor::InlayHint& hint : hints) {
				// 位置小于1的提示没有可用的锚点字符，框架不会为它让出空隙，这里也不能画，否则会和正文重叠。
				if (hint.getPositionInBlock() < 1 or hint.getPositionInBlock() >= block.length()) {
					continue;
				}
				QTextCursor cursor(block);
				cursor.setPosition(block.position() + hint.getPositionInBlock());
				// 空隙在提示位置之前，而光标矩形给出的是提示位置之后的那条边。
				const QRect rightEdge = this->cursorRect(cursor);
				if (not rightEdge.isValid()) {
					continue;
				}
				const qreal gap = YSSCore::__Private__::inlayHintGapWidth(hint, hintFont);
				const QRectF gapRect(rightEdge.left() - gap, rightEdge.top(), gap, rightEdge.height());
				if (not gapRect.intersects(clipRect)) {
					continue;
				}
				this->paintInlayHint(painter, gapRect, hint);
			}
		}
	}

	void InlayHintTextEdit::paintInlayHint(QPainter& painter, const QRectF& gapRect, const YSSCore::Editor::InlayHint& hint) const {
		QColor color = hint.getColor();
		if (not color.isValid()) {
			color = this->palette().color(QPalette::PlaceholderText);
		}
		if (not color.isValid()) {
			color = QColor(140, 140, 140);
		}
		// 背景高度跟随文字本身而不是整行高度，提示看起来才像贴着文字的小徽章。
		const QFontMetricsF metrics(painter.font());
		const QRectF textRect = metrics.boundingRect(hint.getText());
		const qreal contentHeight = qMin(gapRect.height(), textRect.height() + 2);
		const QRectF backgroundRect(gapRect.left() + 1, gapRect.center().y() - contentHeight / 2,
			qMax(0.0, gapRect.width() - 2), contentHeight);
		QColor backgroundColor = color;
		backgroundColor.setAlpha(32);
		painter.setPen(Qt::NoPen);
		painter.setBrush(backgroundColor);
		painter.drawRoundedRect(backgroundRect, 3, 3);
		painter.setPen(color);
		painter.drawText(backgroundRect, Qt::AlignCenter, hint.getText());
	}

	/*!
		\a firstLine 视口内第一行的行号。
		\a lastLine 视口内最后一行的行号。
		\a clipRect 视口坐标下的关注区域。

		\return 文档中是否存在与 \a clipRect 相交的行。
	*/
	bool InlayHintTextEdit::visibleLines(qint32& firstLine, qint32& lastLine, const QRect& clipRect) const {
		QTextDocument* document = this->document();
		if (document == nullptr or document->blockCount() == 0) {
			return false;
		}
		const qint32 bottom = qMin(clipRect.bottom(), this->viewport()->height() - 1);
		const QTextCursor first = this->cursorForPosition(QPoint(0, qMax(0, clipRect.top())));
		const QTextCursor last = this->cursorForPosition(QPoint(0, qMax(0, bottom)));
		if (not first.block().isValid() or not last.block().isValid()) {
			return false;
		}
		firstLine = first.blockNumber();
		lastLine = last.blockNumber();
		return true;
	}

	/*!
		多行建议的插入位置指示：在目标行上沿画一条通栏粗实线，左端是一个指向文字的 > 形状。

		这条线不需要在文本流里占位置，它画在两行之间的空隙上，因此不会改变任何布局。
	*/
	void InlayHintTextEdit::paintSuggestionIndicator(QPainter& painter, const QRect& clipRect) {
		YSSCore::__Private__::TextEditPrivate* d = this->editor->d;
		if (d->SgtKind != YSSCore::__Private__::SuggestionKind::Block) {
			return;
		}
		QTextDocument* document = this->document();
		if (document == nullptr or document->blockCount() == 0) {
			return;
		}
		const qint32 line = qBound(0, d->SgtLine, document->blockCount() - 1);
		const QTextBlock block = document->findBlockByNumber(line);
		if (not block.isValid()) {
			return;
		}
		const QRect lineRect = this->cursorRect(QTextCursor(block));
		if (not lineRect.isValid()) {
			return;
		}
		const int y = lineRect.top();
		if (not clipRect.intersects(QRect(0, y - 6, this->viewport()->width(), 13))) {
			return;
		}
		const QColor color = VISTM->getColor(QStringLiteral("SelectedTextBackground"));
		if (not color.isValid()) {
			return;
		}
		const int left = 2;
		const int chevronWidth = 7;
		const int thickness = 2;
		const int right = this->viewport()->width();
		painter.save();
		painter.setPen(QPen(color, thickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		painter.setBrush(Qt::NoBrush);
		// 箭头与横线画在同一条路径里，从箭头尖端直接接出去，看起来是 >──────── 而不是断开的 > ────。
		QPainterPath path;
		path.moveTo(left, y - 5);
		path.lineTo(left + chevronWidth, y);
		path.lineTo(left, y + 5);
		path.moveTo(left + chevronWidth, y);
		path.lineTo(qMax(left + chevronWidth, right), y);
		painter.drawPath(path);
		painter.restore();
	}

	/*!
		行删除建议：在要删除的每一行上叠一层半透明红底。

		这里按行盒的上下边界取范围，横向直接铺满视口，避免逐字测量；叠色只在绘制层，不动文档。
	*/
	void InlayHintTextEdit::paintRemoveSuggestion(QPainter& painter, const QRect& clipRect) {
		YSSCore::__Private__::TextEditPrivate* d = this->editor->d;
		if (d->SgtKind != YSSCore::__Private__::SuggestionKind::Remove) {
			return;
		}
		QTextDocument* document = this->document();
		if (document == nullptr or document->blockCount() == 0) {
			return;
		}
		const qint32 firstLine = qBound(0, d->SgtLine, document->blockCount() - 1);
		const qint32 lastLine = qBound(firstLine, firstLine + qMax(1, d->SgtRemoveSize) - 1,
			document->blockCount() - 1);
		int top = 0;
		int bottom = -1;
		for (qint32 line = firstLine; line <= lastLine; ++line) {
			const QTextBlock block = document->findBlockByNumber(line);
			if (not block.isValid()) {
				continue;
			}
			const QRect lineRect = this->cursorRect(QTextCursor(block));
			if (not lineRect.isValid()) {
				continue;
			}
			top = bottom < 0 ? lineRect.top() : qMin(top, lineRect.top());
			bottom = qMax(bottom, lineRect.bottom());
		}
		if (bottom < top) {
			return;
		}
		const QRect band = QRect(0, top, this->viewport()->width(), bottom - top + 1).intersected(clipRect);
		if (band.isEmpty()) {
			return;
		}
		painter.fillRect(band, QColor(0xD9, 0x30, 0x30, 64));
	}

	/*!
		pureAdd 行内建议：把建议文字画在目标行行尾，用内联提示的弱化颜色，看起来像还没落盘的补全。

		文字不进入文档，也不占布局空间；接受时才真正追加到行尾。
	*/
	void InlayHintTextEdit::paintPureAddSuggestion(QPainter& painter, const QRect& clipRect) {
		YSSCore::__Private__::TextEditPrivate* d = this->editor->d;
		if (d->SgtKind != YSSCore::__Private__::SuggestionKind::Inline or not d->SgtPureAdd) {
			return;
		}
		QTextDocument* document = this->document();
		if (document == nullptr or document->blockCount() == 0) {
			return;
		}
		const QString text = d->currentSuggestionText();
		if (text.isEmpty()) {
			return;
		}
		const qint32 line = qBound(0, d->SgtLine, document->blockCount() - 1);
		const QTextBlock block = document->findBlockByNumber(line);
		if (not block.isValid()) {
			return;
		}
		QTextCursor endOfLine(block);
		endOfLine.movePosition(QTextCursor::EndOfBlock);
		const QRect lineRect = this->cursorRect(endOfLine);
		if (not lineRect.isValid() or not clipRect.intersects(lineRect)) {
			return;
		}
		QFont font = document->defaultFont();
		const QFontMetricsF metrics(font);
		painter.save();
		painter.setClipRect(clipRect);
		painter.setFont(font);
		painter.setPen(VISTM->getColor(QStringLiteral("InlayHint")));
		painter.drawText(QPointF(lineRect.left() + 6, lineRect.top() + metrics.ascent()), text);
		painter.restore();
	}

	SuggestionWidget::SuggestionWidget(YSSCore::Editor::TextEdit* editor, QWidget* parent)
		: Visindigo::Widgets::BorderFrame(parent) {
		this->editor = editor;
		this->setAttribute(Qt::WA_ShowWithoutActivating);
		this->setFocusPolicy(Qt::NoFocus);
		auto* glass = new Visindigo::Widgets::LiquidGlassEffect(this);
		glass->setBorderRadius(6);
		glass->setBlurRadius(20);
		this->setGraphicsEffect(glass);
		Visindigo::Widgets::LiquidGlassEffect::applyForegroundAlpha(this);

		this->layout = new QVBoxLayout(this);
		this->layout->setContentsMargins(8, 5, 8, 6);
		this->layout->setSpacing(4);

		this->titleLayout = new QHBoxLayout();
		this->titleLayout->setContentsMargins(0, 0, 0, 0);
		this->titleLayout->setSpacing(6);

		this->indexLabel = new QLabel(this);
		auto titleFont = this->font();
		titleFont.setPointSizeF(titleFont.pointSizeF() * 0.85);
		this->indexLabel->setFont(titleFont);
		this->titleLayout->addWidget(this->indexLabel);

		// 小标题居中，用两侧的弹簧和左右元素隔开；目标行不在视口内时它会变成"滚动到行 xxx"。
		this->titleLayout->addStretch(1);
		this->titleLabel = new QLabel(this);
		this->titleLabel->setFont(titleFont);
		this->titleLabel->setAlignment(Qt::AlignCenter);
		this->titleLayout->addWidget(this->titleLabel);

		this->arrowLabel = new QLabel(this);
		this->arrowLabel->setFixedSize(14, 14);
		this->arrowLabel->setVisible(false);
		this->titleLayout->addWidget(this->arrowLabel);
		this->titleLayout->addStretch(1);

		// 接受/拒绝用图标而不是文字：图标来自应用的图标字体，按语义着色（接受绿、拒绝红）。
		this->rejectButton = new QToolButton(this);
		this->acceptButton = new QToolButton(this);
		const QList<QToolButton*> buttons = { this->rejectButton, this->acceptButton };
		for (QToolButton* button : buttons) {
			button->setFocusPolicy(Qt::NoFocus);
			button->setAutoRaise(true);
			button->setToolButtonStyle(Qt::ToolButtonIconOnly);
			button->setFixedSize(20, 18);
			button->setIconSize(QSize(13, 13));
			this->titleLayout->addWidget(button);
		}
		if (not VIApp->getIconFont().family().isEmpty()) {
			this->acceptButton->setIcon(VIApp->getNamedFontIcon(QStringLiteral("Accept"), 64,
				{ QColor(0x3F, 0xA4, 0x5B) }));
			this->rejectButton->setIcon(VIApp->getNamedFontIcon(QStringLiteral("Cancel"), 64,
				{ QColor(0xD9, 0x53, 0x4F) }));
			this->acceptButton->setToolTip(VITRL("YSS::editor.textEdit.suggestion.accept"));
			this->rejectButton->setToolTip(VITRL("YSS::editor.textEdit.suggestion.reject"));
		}
		this->layout->addLayout(this->titleLayout);

		this->contentLabel = new QLabel(this);
		this->contentLabel->setTextInteractionFlags(Qt::NoTextInteraction);
		// 建议内容是要插入文档的原文，不能按富文本解析，否则含 < 或 & 的内容会被吃掉。
		this->contentLabel->setTextFormat(Qt::PlainText);
		this->contentLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		this->layout->addWidget(this->contentLabel);

		connect(this->acceptButton, &QToolButton::clicked, this, [this]() { emit accepted(); });
		connect(this->rejectButton, &QToolButton::clicked, this, [this]() { emit rejected(); });
	}

	/*!
		\a index 当前建议的序号，从 1 开始；\a total 建议总数。
		\a index 小于等于 0 时不显示序号（行删除建议没有多个候选）。
	*/
	void SuggestionWidget::setIndex(qint32 index, qint32 total) {
		if (index <= 0) {
			this->indexLabel->clear();
			this->indexLabel->setVisible(false);
			return;
		}
		this->indexLabel->setText(QStringLiteral("%1/%2").arg(index).arg(total));
		this->indexLabel->setVisible(true);
	}

	/*!
		\a title 显示在序号与按钮之间的小标题。
	*/
	void SuggestionWidget::setTitle(const QString& title) {
		this->titleLabel->setText(title);
		this->titleLabel->setVisible(not title.isEmpty());
	}

	/*!
		\a direction 0 隐藏箭头，-1 向上（目标行在视口上方），1 向下（目标行在视口下方）。
		箭头用选中色着色，和"滚动到该行"的语义保持一致。
	*/
	void SuggestionWidget::setScrollArrow(qint32 direction) {
		if (direction == 0) {
			this->arrowLabel->clear();
			this->arrowLabel->setVisible(false);
			return;
		}
		const QColor color = VISTM->getColor(QStringLiteral("SelectedTextBackground"));
		if (direction < 0) {
			if (this->scrollUpIcon.isNull()) {
				this->scrollUpIcon = VIApp->getNamedFontIcon(QStringLiteral("ChevronUp"), 64, { color });
			}
			this->arrowLabel->setPixmap(this->scrollUpIcon.pixmap(14, 14));
		}
		else {
			if (this->scrollDownIcon.isNull()) {
				this->scrollDownIcon = VIApp->getNamedFontIcon(QStringLiteral("ChevronDown"), 64, { color });
			}
			this->arrowLabel->setPixmap(this->scrollDownIcon.pixmap(14, 14));
		}
		this->arrowLabel->setVisible(true);
	}

	/*!
		\a content 建议正文；为空时隐藏正文区（行删除建议没有正文）。
	*/
	void SuggestionWidget::setContent(const QString& content) {
		if (content.isEmpty()) {
			this->contentLabel->clear();
			this->contentLabel->setVisible(false);
			return;
		}
		this->contentLabel->setText(content);
		this->contentLabel->setVisible(true);
		// 建议的内容就是要放进文档的文字，因此用正文的字体来显示，看起来才和结果一致。
		this->contentLabel->setFont(this->editor->getTextFont());
	}

	/*!
		按当前内容重新计算浮窗大小：宽度跟随最长的一行（标题行也算），高度跟随行数，宽度上限为视口的六成。
	*/
	void SuggestionWidget::applySize() {
		this->layout->activate();
		const QSize hint = this->layout->sizeHint();
		const QWidget* area = this->parentWidget();
		const int areaWidth = area ? area->width() : this->editor->width();
		const int maxWidth = qMax(220, int(areaWidth * 0.6));
		const int width = qBound(160, hint.width() + 4, maxWidth);
		this->setFixedSize(width, qMax(40, hint.height()));
		this->update();
	}

	void SuggestionWidget::onThemeChanged() {
		this->update();
	}
}
