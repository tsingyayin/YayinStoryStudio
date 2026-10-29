#include <QtCore/qcoreapplication.h>
#include <QtCore/qtimer.h>
#include <algorithm>
#include <QtCore/qvarLengthArray.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpalette.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextlayout.h>
#include <QtGui/qtextobject.h>
#include <QtWidgets/qapplication.h>
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
#include "Editor/InlayHintHoverWidget.h"
#include "Editor/InlayHintManager.h"
#include "Editor/InlayHintProvider.h"
#include "Editor/SyntaxHighlighter.h"
#include "Editor/VirtualFilePath.h"
#include "Editor/private/InlayHint_p.h"
#include "Editor/private/TextEdit_p.h"
#include "Editor/TextEdit.h"

namespace YSSCore::__Private__ {
	// 行号与编辑区之间的留白，以及行号栏至少预留的位数
	// （行数少的文件也留够，免得多一位数字就贴边）
	constexpr qint32 LineNumberBarPadding = 12;
	constexpr qint32 LineNumberBarMinDigits = 6;

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

	// 缩略图按"字符格"渲染：宽度方向一格 1px，高度方向一行 2px（与 VS Code 的
	// BASE_CHAR_WIDTH / BASE_CHAR_HEIGHT 一致）。每个字符用一张 1x2 的字形掩码着色，
	// 掩码的灰度取自真实字形的墨迹覆盖率，因此在缩略图上能看出"字"，而不是一根实心横条。
	static constexpr qint32 OverviewGutterWidth = 10;
	static constexpr qint32 OverviewCharWidth = 1;
	static constexpr qint32 OverviewLineHeight = 2;
	// 文档很长时按比例算出来的视口指示器可能只有几个像素高，鼠标很难抓住。
	static constexpr qint32 OverviewViewportIndicatorMinHeight = 20;

	static qint32 overviewViewportIndicatorHeight(qint32 viewHeight, qint32 pageStep, qint32 totalRange) {
		if (viewHeight <= 0 or totalRange <= 0) {
			return OverviewViewportIndicatorMinHeight;
		}
		const qint32 proportional = viewHeight * pageStep / totalRange;
		return qMin(viewHeight, qMax(proportional, OverviewViewportIndicatorMinHeight));
	}
	// 字形采样尺寸：先把字符画进 10x16 的格子，再降采样到 1x2。
	static constexpr qint32 OverviewSampleWidth = 10;
	static constexpr qint32 OverviewSampleHeight = 16;
	static constexpr char16_t OverviewFirstChar = 32;
	static constexpr char16_t OverviewLastChar = 126;
	// 表里最后一项是"未知字符"，非 ASCII 走与 VS Code 相同的映射时会用到它。
	static constexpr qint32 OverviewGlyphCount = OverviewLastChar - OverviewFirstChar + 2;
	static constexpr char16_t OverviewUnknownChar = 0xFFFD;

	// 缩略图的一行：在控件里的 y 坐标，以及这一行代表哪个文本块。
	struct OverviewRowSlot {
		qint32 y;
		qint32 blockNumber;
	};

	// 把 \a coverage 覆盖率下的 \a color 写到 \a line 的第 \a x 个像素上。
	// 图像是预乘 ARGB，用 qPremultiply 生成像素值，与平台字节序无关。
	static void writeInk(uchar* line, qint32 x, QRgb color, qint32 coverage) {
		const qint32 alpha = coverage * qAlpha(color) / 255;
		if (alpha <= 0) {
			return;
		}
		reinterpret_cast<quint32*>(line)[x] =
			qPremultiply(qRgba(qRed(color), qGreen(color), qBlue(color), alpha));
	}

	// 全角字符占两格，中文在缩略图里的宽度比例才和正文一致。
	static bool isFullWidthChar(char16_t code) {
		return (code >= 0x1100 and code <= 0x115F)      // 谚文字母
			or (code >= 0x2E80 and code <= 0xA4CF)      // 部首扩展、假名、汉字
			or (code >= 0xAC00 and code <= 0xD7A3)      // 谚文音节
			or (code >= 0xF900 and code <= 0xFAFF)      // 兼容汉字
			or (code >= 0xFE30 and code <= 0xFE6F)      // 兼容形式
			or (code >= 0xFF00 and code <= 0xFF60)      // 全角 ASCII
			or (code >= 0xFFE0 and code <= 0xFFE6);
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

	void DocumentOverviewLabel::mousePressEvent(QMouseEvent* event) {
		if (event->button() == Qt::LeftButton and this->editor != nullptr) {
			auto* scrollBar = this->editor->d->Text->verticalScrollBar();
			const int maximum = scrollBar->maximum();
			const int pageStep = scrollBar->pageStep();
			const int totalRange = maximum + pageStep;
			const int h = this->height();
			const int indicatorH = overviewViewportIndicatorHeight(h, pageStep, totalRange);
			if (maximum > 0 and h > indicatorH) {
				// 单击缩略图时把点击位置放到视口中心，而不是让视口顶边贴住点击位置。
				const qreal ratio = (event->position().y() - indicatorH / 2.0) / (h - indicatorH);
				const int newValue = qBound(scrollBar->minimum(),
					qRound(ratio * maximum), maximum);
				scrollBar->setValue(newValue);
				event->accept();
				return;
			}
		}
		QWidget::mousePressEvent(event);
	}

	void DocumentOverviewLabel::wheelEvent(QWheelEvent* event) {
		if (this->editor == nullptr) {
			QWidget::wheelEvent(event);
			return;
		}
		const int delta = event->angleDelta().y();
		if (delta != 0) {
			auto* scrollBar = this->editor->d->Text->verticalScrollBar();
			const int lines = qMax(1, QApplication::wheelScrollLines());
			const int singleStep = qMax(1, scrollBar->singleStep());
			const int amount = qRound(delta * lines * singleStep / 120.0);
			if (amount != 0) {
				scrollBar->setValue(scrollBar->value() - amount);
			}
			event->accept();
			return;
		}
		QWidget::wheelEvent(event);
	}

	QSize DocumentOverviewLabel::sizeHint() const {
		return QSize(80, 0);
	}

	/*!
		构建字形掩码表：把 ASCII 可见字符按 16px 粗体画进 10x16 的格子，
		再把每个格子按纵向对半降采样成 2 个字节，值表示该处的墨迹覆盖率。

		表只在字体族变化时重建一次，渲染时直接查表，不做任何字体光栅化。
	*/
	void DocumentOverviewLabel::ensureGlyphAtlas() {
		const QString family = this->document != nullptr ? this->document->defaultFont().family() : QString();
		if (family == this->glyphFamily and this->glyphData.size() == OverviewGlyphCount * OverviewLineHeight) {
			return;
		}

		QImage sample(OverviewGlyphCount * OverviewSampleWidth, OverviewSampleHeight, QImage::Format_ARGB32);
		sample.fill(Qt::transparent);
		{
			QPainter painter(&sample);
			QFont font(family);
			font.setPixelSize(OverviewSampleHeight);
			font.setBold(true);
			painter.setFont(font);
			painter.setPen(Qt::white);
			for (qint32 i = 0; i < OverviewGlyphCount; ++i) {
				const char16_t code = (i < OverviewGlyphCount - 1)
					? char16_t(OverviewFirstChar + i)
					: OverviewUnknownChar;
				painter.drawText(QRect(i * OverviewSampleWidth, 0, OverviewSampleWidth, OverviewSampleHeight),
					Qt::AlignHCenter | Qt::AlignVCenter, QString(QChar(code)));
			}
		}

		QByteArray data(OverviewGlyphCount * OverviewLineHeight, '\0');
		qint32 brightest = 0;
		for (qint32 i = 0; i < OverviewGlyphCount; ++i) {
			for (qint32 row = 0; row < OverviewLineHeight; ++row) {
				const qint32 firstY = row * OverviewSampleHeight / OverviewLineHeight;
				const qint32 lastY = (row + 1) * OverviewSampleHeight / OverviewLineHeight;
				qint32 sum = 0;
				qint32 samples = 0;
				for (qint32 y = firstY; y < lastY; ++y) {
					const QRgb* line = reinterpret_cast<const QRgb*>(sample.constScanLine(y));
					for (qint32 x = i * OverviewSampleWidth; x < (i + 1) * OverviewSampleWidth; ++x) {
						sum += qAlpha(line[x]);
						++samples;
					}
				}
				const qint32 value = samples > 0 ? sum / samples : 0;
				data[i * OverviewLineHeight + row] = char(value);
				brightest = qMax(brightest, value);
			}
		}
		// 降采样到 1x2 之后墨迹很淡，按最亮的字形归一化，字与字之间才有区分度。
		if (brightest > 0) {
			for (char& value : data) {
				value = char(qMin(255, int(uchar(value)) * 255 / brightest));
			}
		}
		this->glyphFamily = family;
		this->glyphData = data;
	}

	/*!
		取 \a ch 对应的掩码（OverviewLineHeight 个字节）。

		ASCII 直接对应；其余字符（中文、全角标点等）没有现成的字形表，按 VS Code 的
		做法把它们映射到某一个 ASCII 字形上（`(码位 - 32 + 表长) % 表长`）。这样不同的
		汉字会得到不同的墨色深浅——实测 VS Code 里"永"映射到"8"、"一"映射到"`"、
		"口"映射到"C"，正是中文段落看起来像文字而不是一片实心的原因。
	*/
	const uchar* DocumentOverviewLabel::glyphMask(char16_t ch) const {
		static const uchar fullInk[OverviewLineHeight] = { 255, 255 };
		if (this->glyphData.isEmpty()) {
			return fullInk;
		}
		qint32 index = qint32(ch) - OverviewFirstChar;
		if (index < 0 or index >= OverviewGlyphCount) {
			index = (index + OverviewGlyphCount) % OverviewGlyphCount;
			if (index < 0) {
				index += OverviewGlyphCount;
			}
		}
		return reinterpret_cast<const uchar*>(this->glyphData.constData()) + index * OverviewLineHeight;
	}

	/*!
		把 \a block 画到 \a y 这一行（占 OverviewLineHeight 行像素）。

		这里只处理这一块自己的字符与格式，代价与本块长度成正比；调用方负责挑出
		真正需要画的那几块，因此整体代价与文档行数无关。
	*/
	void DocumentOverviewLabel::renderRow(qint32 y, const QTextBlock& block) {
		const qint32 columns = (this->width() - OverviewGutterWidth) / OverviewCharWidth;
		if (columns <= 0 or y < 0 or y + OverviewLineHeight > this->backBuffer.height()) {
			return;
		}
		const QString text = block.text();
		if (text.isEmpty()) {
			return;
		}

		// 每个字符的颜色：默认前景色，再被格式区间覆盖。全角字符占两格，所以按 2 倍宽松取值。
		const qint32 textLength = qMin(text.size(), columns * 2);
		const QRgb defaultColor = this->palette().color(QPalette::Text).rgba();
		QVarLengthArray<QRgb, 256> colors;
		colors.resize(textLength);
		std::fill(colors.begin(), colors.end(), defaultColor);
		if (QTextLayout* layout = block.layout(); layout != nullptr) {
			for (const auto& range : layout->formats()) {
				const QBrush foreground = range.format.foreground();
				if (foreground.style() == Qt::NoBrush or not foreground.color().isValid()) {
					continue;
				}
				const qint32 first = qMax(range.start, 0);
				const qint32 last = qMin(range.start + range.length, textLength);
				for (qint32 i = first; i < last; ++i) {
					colors[i] = foreground.color().rgba();
				}
			}
		}

		uchar* firstLine = this->backBuffer.scanLine(y);
		uchar* secondLine = this->backBuffer.scanLine(y + 1);
		const qint32 limit = this->width();
		qint32 cell = OverviewGutterWidth;
		for (qint32 i = 0; i < textLength and cell < limit; ++i) {
			const char16_t code = text.at(i).unicode();
			const qint32 cellCount = isFullWidthChar(code) ? 2 : 1;
			// 空白不画：缩略图是"文字分布"的示意，缩进和词间的空隙要能看出来。
			if (code != u' ' and code != u'\t') {
				const uchar* mask = this->glyphMask(code);
				const QRgb color = colors[i];
				for (qint32 offset = 0; offset < cellCount; ++offset) {
					const qint32 x = cell + offset;
					if (x >= limit) {
						break;
					}
					// 掩码的两个字节分别是上下两行的墨迹覆盖率。
					writeInk(firstLine, x, color, mask[0]);
					writeInk(secondLine, x, color, mask[1]);
				}
			}
			cell += cellCount * OverviewCharWidth;
		}
	}

	/*!
		重绘缩略图。

		只枚举"会被看到的那些块"：缩略图共 rows 行，行 r 对应块 r*lineCount/rows，
		其余行根本不会出现在缩略图上，因此不必为它们算颜色——5 万行的文档也只处理
		几百块，代价与文档长度无关。
	*/
	void DocumentOverviewLabel::paintEvent(QPaintEvent* event) {
		QWidget::paintEvent(event);
		if (this->document == nullptr) {
			return;
		}

		const qint32 lineCount = this->document->blockCount();
		const qint32 rows = this->height() / OverviewLineHeight;
		if (lineCount <= 0 or rows <= 0 or this->width() <= OverviewGutterWidth) {
			return;
		}

		if (this->backBuffer.size() != this->size()) {
			this->backBuffer = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
		}
		this->backBuffer.fill(Qt::transparent);
		this->ensureGlyphAtlas();

		// 行数少于缩略图行数时同一个块会连续占好几行，此时只画一次，避免同一行被涂粗。
		QVarLengthArray<OverviewRowSlot, 512> rowSlots;
		qint32 previous = -1;
		for (qint32 r = 0; r < rows; ++r) {
			const qint32 target = qint32(qint64(r) * lineCount / rows);
			if (target == previous) {
				continue;
			}
			previous = target;
			rowSlots.append(OverviewRowSlot{ r * OverviewLineHeight, target });
		}

		for (const OverviewRowSlot& slot : rowSlots) {
			const QTextBlock block = this->document->findBlockByNumber(slot.blockNumber);
			if (block.isValid()) {
				this->renderRow(slot.y, block);
			}
		}

		QPainter painter(this);
		painter.drawImage(0, 0, this->backBuffer);

		// 错误行标记
		if (not this->errorLines.isEmpty()) {
			painter.setPen(QPen(Qt::red, 4));
			for (const OverviewRowSlot& slot : rowSlots) {
				if (not this->errorLines.contains(slot.blockNumber)) {
					continue;
				}
				const qint32 lineTop = qMax(slot.y - 1, 0);
				const qint32 lineBottom = qMin(slot.y + OverviewLineHeight, this->height() - 1);
				painter.drawLine(3, lineTop, 3, lineBottom);
			}
		}
	}

	void DocumentOverviewLabel::onContentsChange(int position, int charsRemoved, int charsAdded) {
		Q_UNUSED(position);
		Q_UNUSED(charsRemoved);
		Q_UNUSED(charsAdded);
		this->update();
		updateViewportIndicator();
	}

	void DocumentOverviewLabel::recalculateAll() {
		// 缩略图不再缓存全文颜色，重绘时按需现算，这里只需要标脏。
		this->update();
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
		const int indicatorH = overviewViewportIndicatorHeight(h, pageStep, totalRange);
		const int trackHeight = qMax(0, h - indicatorH);
		const int indicatorY = maximum > 0 and trackHeight > 0
			? trackHeight * scrollBar->value() / maximum
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
					const int indicatorH = overviewViewportIndicatorHeight(h, pageStep, totalRange);

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

	InlayHintTextEdit::InlayHintTextEdit(YSSCore::Editor::TextEdit* editor, QWidget* parent) : QPlainTextEdit(parent) {
		this->editor = editor;
	}

	QTextBlock InlayHintTextEdit::getFirstVisibleBlock() const {
		return this->firstVisibleBlock();
	}

	QRectF InlayHintTextEdit::getBlockBoundingGeometry(const QTextBlock& block) const {
		return this->blockBoundingGeometry(block);
	}

	QRectF InlayHintTextEdit::getBlockBoundingRect(const QTextBlock& block) const {
		return this->blockBoundingRect(block);
	}

	QPointF InlayHintTextEdit::getContentOffset() const {
		return this->contentOffset();
	}

	LineNumberBar::LineNumberBar(InlayHintTextEdit* editor, QWidget* parent) : QWidget(parent) {
		this->editor = editor;
		setObjectName(QStringLiteral("LineNumberBar"));
		setFocusPolicy(Qt::NoFocus);
		setAttribute(Qt::WA_OpaquePaintEvent, true);
		refreshMetrics();
	}

	/*!
		行数变化时只需要更新宽度（位数可能变了）并重绘，不再维护任何文本块。

		打开大文件时 blockCountChanged 会逐块触发几万次，因此位数没变时只标脏，
		连字体度量都不必重算。
	*/
	void LineNumberBar::updateLineCount(qint32 count) {
		lineCount = qMax(1, count);
		const qint32 digits = qMax<qint32>(LineNumberBarMinDigits, QString::number(lineCount).length());
		if (digits == cachedDigits) {
			update();
			return;
		}
		cachedDigits = digits;
		refreshMetrics();
	}

	/*!
		字体/字号变化后行号宽度会变，因此重新按当前字体算一次宽度。

		宽度只由位数和字体决定，不再设上限：以前那道上限会在行数或字号一大就把数字裁掉。
	*/
	void LineNumberBar::refreshMetrics() {
		const QFontMetrics metrics(editor->document()->defaultFont());
		const qint32 digits = qMax<qint32>(LineNumberBarMinDigits, QString::number(lineCount).length());
		cachedDigits = digits;
		const qint32 digitWidth = qMax(metrics.horizontalAdvance(QLatin1Char('0')),
			metrics.horizontalAdvance(QLatin1Char('9')));
		const qint32 width = digitWidth * digits + LineNumberBarPadding;
		if (width != this->width()) {
			this->setFixedWidth(width);
		}
		update();
	}

	/*!
		只画视口里能看见的那几行：从 firstVisibleBlock 开始往下走，直到超出底边为止，
		因此代价与文档总行数无关。

		QPlainTextEdit 的滚动条以"块"为单位，块几何是 viewport 坐标（首个可见块的 y 由
		contentOffset() 给出，通常是个负值，表示它已经卷出视口多少）。行号栏与编辑区是
		同一父控件下的兄弟，两者的 viewport 顶边相差一个固定偏移，按 pos() 换算即可。
	*/
	void LineNumberBar::paintEvent(QPaintEvent* event) {
		QPainter painter(this);
		painter.fillRect(event->rect(), editor->palette().color(QPalette::Base));

		QTextDocument* document = editor->document();
		if (document == nullptr or document->blockCount() == 0) {
			return;
		}
		QTextBlock block = editor->getFirstVisibleBlock();
		if (not block.isValid()) {
			return;
		}
		// 行号栏与编辑区是编辑器控件里的兄弟，用各自 pos() 换算即可，不必依赖全局映射。
		qint32 viewportTop = editor->viewport()->pos().y();
		if (this->parentWidget() != nullptr && editor->parentWidget() == this->parentWidget()) {
			viewportTop += editor->pos().y() - this->pos().y();
		}
		const qint32 clipTop = event->rect().top() - viewportTop;
		const qint32 clipBottom = event->rect().bottom() - viewportTop;

		painter.setPen(editor->palette().color(QPalette::Text));
		painter.setFont(document->defaultFont());
		const qreal barWidth = this->width();
		qint32 top = qRound(editor->getBlockBoundingGeometry(block).translated(editor->getContentOffset()).top());
		qint32 bottom = top + qRound(editor->getBlockBoundingRect(block).height());
		for (; block.isValid() && top <= clipBottom; block = block.next()) {
			if (block.isVisible() && bottom >= clipTop) {
				// 横向范围取行号栏自己的宽度：块的宽度是整个文档的宽度，直接用它右对齐会画到控件外面。
				const QRectF numberRect(0, viewportTop + top, barWidth, bottom - top);
				painter.drawText(numberRect.adjusted(0, 0, -LineNumberBarPadding, 0),
					Qt::AlignRight | Qt::AlignVCenter, QString::number(block.blockNumber() + 1));
			}
			top = bottom;
			bottom = top + qRound(editor->getBlockBoundingRect(block.next()).height());
		}
	}

	/*!
		行号栏本身不滚动，滚轮事件转交给编辑区，保持鼠标停在行号上也能滚动文档的旧行为。
	*/
	void LineNumberBar::wheelEvent(QWheelEvent* event) {
		QCoreApplication::sendEvent(editor->viewport(), event);
	}

	/*!
		绘制文本本身之后，再把内联提示画到文本流为它让出的空隙里。

		提示的空隙由 YSSCore::Editor::SyntaxHighlighter 通过字距注入产生，那个字距只存在于布局层，
		并不在文档里，所以文档内容、保存结果和撤销栈都不受影响，提示的文字需要由这里自己画上去。
	*/
	void InlayHintTextEdit::paintEvent(QPaintEvent* event) {
		QPlainTextEdit::paintEvent(event);
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
		if (this->editor != nullptr) {
			for (YSSCore::Editor::InlayHintHoverWidget* widget : this->editor->d->InlayHintHoverWidgets) {
				if (widget != nullptr and widget->isVisible() and widget->getBlockNumber() == lineNumber) {
					this->editor->d->hideInlayHintHover();
					break;
				}
			}
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
		this->editor->d->hideInlayHintHover();
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
		const QFont baseFont = document->defaultFont();
		const QFont hintFont = YSSCore::__Private__::inlayHintFont(baseFont,
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
				const qreal gap = YSSCore::__Private__::inlayHintGapWidth(hint, baseFont, hintFont);
				if (gap <= 0) {
					continue;
				}
				const QRectF gapRect(rightEdge.left() - gap, rightEdge.top(), gap, rightEdge.height());
				if (not gapRect.intersects(clipRect)) {
					continue;
				}
				YSSCore::Editor::InlayHintProvider* provider = nullptr;
				YSSCore::Editor::VirtualFilePath multiPath(hint.getMulti());
				if (multiPath.isValid()) {
					provider = YSSIHM->getProvider(multiPath.getExt());
				}
				if (provider != nullptr) {
					Visindigo::Utility::JsonConfig para = multiPath.getParams();
					const qint32 side = qRound(gap);
					QImage& image = provider->drawInlayHint(side, side,
						this->editor->getUri(), multiPath.getFileName(), para);
					if (not image.isNull()) {
						const QSizeF targetSize = QSizeF(image.size()).scaled(gapRect.size(), Qt::KeepAspectRatio);
						const QRectF targetRect(gapRect.center().x() - targetSize.width() / 2,
							gapRect.center().y() - targetSize.height() / 2,
							targetSize.width(), targetSize.height());
						painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
						painter.drawImage(targetRect, image);
						continue;
					}
				}
				if (not hint.getText().isEmpty()) {
					this->paintInlayHint(painter, gapRect, hint);
				}
			}
		}
	}

	bool InlayHintTextEdit::getInlayHintAt(const QPoint& viewportPos, qint32& lineNumber, qint32& column, QString& multi) const {
		if (this->editor == nullptr or not this->editor->isInlayHintVisible()) {
			return false;
		}
		YSSCore::Editor::SyntaxHighlighter* highlighter = this->editor->getSyntaxHighlighter();
		QTextDocument* document = this->document();
		if (highlighter == nullptr or document == nullptr) {
			return false;
		}
		const QTextBlock block = this->cursorForPosition(viewportPos).block();
		if (not block.isValid()) {
			return false;
		}
		const QFont baseFont = document->defaultFont();
		const QFont hintFont = YSSCore::__Private__::inlayHintFont(baseFont,
			this->editor->getInlayHintFontScale());
		const QList<YSSCore::Editor::InlayHint> hints = highlighter->getInlayHints(block.blockNumber());
		for (const YSSCore::Editor::InlayHint& hint : hints) {
			QString providerID;
			if (not YSSCore::__Private__::inlayHintUsesProvider(hint, &providerID)) {
				continue;
			}
			if (hint.getPositionInBlock() < 1 or hint.getPositionInBlock() >= block.length()) {
				continue;
			}
			QTextCursor hintCursor(block);
			hintCursor.setPosition(block.position() + hint.getPositionInBlock());
			const QRect rightEdge = this->cursorRect(hintCursor);
			if (not rightEdge.isValid()) {
				continue;
			}
			const qreal gap = YSSCore::__Private__::inlayHintGapWidth(hint, baseFont, hintFont);
			if (gap <= 0) {
				continue;
			}
			const QRectF gapRect(rightEdge.left() - gap, rightEdge.top(), gap, rightEdge.height());
			if (gapRect.adjusted(-2, -2, 2, 2).contains(viewportPos)) {
				lineNumber = block.blockNumber();
				column = hint.getPositionInBlock();
				multi = hint.getMulti();
				return true;
			}
		}
		return false;
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
		QString text = hint.getText();
		if (metrics.horizontalAdvance(text) > backgroundRect.width()) {
			text = metrics.elidedText(text, Qt::ElideRight, backgroundRect.width());
		}
		painter.drawText(backgroundRect, Qt::AlignCenter, text);
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
		glass->setLiquidDistortRadius(5);
		glass->setLiquidDistortStrength(10);
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
