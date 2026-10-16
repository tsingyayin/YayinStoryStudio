#include <QtCore/qtimer.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpalette.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextlayout.h>
#include <QtGui/qtextobject.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qstyleoption.h>
#include <General/TranslationHost.h>
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
		for (qint32 line : lines) {
			highlighter->rehighlightBlock(document->findBlockByNumber(line));
		}
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
}