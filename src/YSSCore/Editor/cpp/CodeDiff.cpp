#include <algorithm>
#include <cmath>
#include <QtCore/qtimer.h>
#include <QtGui/qabstracttextdocumentlayout.h>
#include <QtGui/qfontdatabase.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpen.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qtextobject.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qtextedit.h>
#include <General/TranslationHost.h>
#include <General/YSSURI.h>
#include <General/YSSUniqueFileSystem.h>
#include <Utility/JsonConfig.h>
#include "Editor/CodeDiff.h"
#include "Editor/private/CodeDiff_p.h"

namespace YSSCore::Editor {
	using namespace YSSCore::General;

	CodeDiffProvider::~CodeDiffProvider() {}

	static QStringList splitLines(const QByteArray& data);

	/*!
		\since YSS 0.18.0
		\a oldLines 旧侧的文本行。
		\a newLines 新侧的文本行。
		return 相异范围列表，按旧侧起始行升序，元素为
		(旧侧起始行, 旧侧行数, 新侧起始行, 新侧行数)。

		使用 Myers 最短编辑脚本算法。差异数超过内部上限时退化为整篇替换，
		以保证耗时和内存可控。
	*/
	QList<CodeDiffRange> BuiltinCodeDiffProvider::compute(const QStringList& oldLines, const QStringList& newLines) {
		const qint32 n = oldLines.size();
		const qint32 m = newLines.size();
		QList<CodeDiffRange> result;
		if (n == 0 && m == 0) {
			return result;
		}
		if (n == 0) {
			result.append({ 0, 0, 0, m });
			return result;
		}
		if (m == 0) {
			result.append({ 0, n, 0, 0 });
			return result;
		}

		const qint32 maxD = qMin(n + m, 2048);
		const qint32 offset = n + m;
		QList<qint32> vFull(2 * (n + m) + 1, 0);
		// trace[d] 保存深度 d 时 [-d, d] 范围内的 V 值，避免每层复制整个数组。
		QList<QList<qint32>> trace;

		bool found = false;
		qint32 finalD = 0;
		for (qint32 d = 0; d <= maxD && !found; ++d) {
			for (qint32 k = -d; k <= d; k += 2) {
				qint32 x = 0;
				if (k == -d || (k != d && vFull[k - 1 + offset] < vFull[k + 1 + offset])) {
					x = vFull[k + 1 + offset];
				}
				else {
					x = vFull[k - 1 + offset] + 1;
				}
				qint32 y = x - k;
				while (x < n && y < m && oldLines[x] == newLines[y]) {
					++x;
					++y;
				}
				vFull[k + offset] = x;
				if (x >= n && y >= m) {
					finalD = d;
					found = true;
					break;
				}
			}
			QList<qint32> slice;
			slice.reserve(2 * d + 1);
			for (qint32 k = -d; k <= d; ++k) {
				slice.append(vFull[k + offset]);
			}
			trace.append(slice);
		}
		if (!found) {
			result.append({ 0, n, 0, m });
			return result;
		}

		QList<char> ops;
		ops.reserve(n + m);
		qint32 x = n;
		qint32 y = m;
		for (qint32 d = finalD; d > 0; --d) {
			const QList<qint32>& previous = trace[d - 1];
			const qint32 k = x - y;
			qint32 prevK = 0;
			if (k == -d || (k != d && previous[k - 1 + (d - 1)] < previous[k + 1 + (d - 1)])) {
				prevK = k + 1;
			}
			else {
				prevK = k - 1;
			}
			const qint32 prevX = previous[prevK + (d - 1)];
			const qint32 prevY = prevX - prevK;
			while (x > prevX && y > prevY) {
				ops.append('=');
				--x;
				--y;
			}
			ops.append(x == prevX ? '+' : '-');
			x = prevX;
			y = prevY;
		}
		while (x > 0 && y > 0) {
			ops.append('=');
			--x;
			--y;
		}
		std::reverse(ops.begin(), ops.end());

		qint32 oldPos = 0;
		qint32 newPos = 0;
		qint32 i = 0;
		const qint32 opCount = ops.size();
		while (i < opCount) {
			if (ops[i] == '=') {
				++oldPos;
				++newPos;
				++i;
				continue;
			}
			const qint32 oldStart = oldPos;
			const qint32 newStart = newPos;
			while (i < opCount && ops[i] != '=') {
				if (ops[i] == '-') {
					++oldPos;
				}
				else {
					++newPos;
				}
				++i;
			}
			result.append({ oldStart, oldPos - oldStart, newStart, newPos - newStart });
		}
		return result;
	}

	DiffPane::DiffPane(QWidget* parent) : QTextEdit(parent) {
		setReadOnly(true);
		setUndoRedoEnabled(false);
		setLineWrapMode(QTextEdit::NoWrap);
		setFrameShape(QFrame::NoFrame);
		document()->setDocumentMargin(4);
		CodeFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
		Bar = new LineNumberBar(this);
		Bar->hide();
		// 滚动、重排或内容变化都可能让可见块变化，行号栏跟着重绘。
		QObject::connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
			if (Bar) {
				Bar->update();
			}
			});
		QObject::connect(document(), &QTextDocument::contentsChanged, this, [this]() {
			if (Bar) {
				Bar->update();
			}
			});
		setCodeFont(CodeFont);
	}

	// 45° 阴影：线族 (全局 x + y = k * step)，两个绘制区域传入各自的全局 x 原点，
	// 这样正文区和行号区的斜线可以无缝衔接。
	// 颜色跟随正文色而不是 Mid：Mid 在部分主题里和背景几乎同色，会淡到看不见。
	static QColor hatchLineColor(const QPalette& palette) {
		QColor color = palette.color(QPalette::Text);
		color.setAlpha(110);
		return color;
	}

	static QColor hatchFillColor(const QPalette& palette) {
		QColor color = palette.color(QPalette::Text);
		color.setAlpha(24);
		return color;
	}

	static void paintDiagonalHatch(QPainter& painter, const QRectF& rect, qreal originX,
		const QColor& fillColor, const QColor& lineColor) {
		if (rect.isEmpty()) {
			return;
		}
		painter.fillRect(rect, fillColor);
		const qreal step = 8.0;
		const qreal left = rect.left() + originX;
		const qreal right = rect.right() + originX;
		const qreal top = rect.top();
		const qreal bottom = rect.bottom();
		painter.setPen(QPen(lineColor, 1));
		const qreal first = std::floor((left + top) / step);
		const qreal last = std::ceil((right + bottom) / step);
		for (qreal k = first; k <= last; k += 1.0) {
			const qreal c = k * step;
			const qreal yStart = qMax(top, c - right);
			const qreal yEnd = qMin(bottom, c - left);
			if (yStart > yEnd) {
				continue;
			}
			painter.drawLine(QPointF(c - yStart - originX, yStart), QPointF(c - yEnd - originX, yEnd));
		}
	}

	LineNumberBar::LineNumberBar(DiffPane* owner) : QWidget(owner), Owner(owner) {
		setAttribute(Qt::WA_TransparentForMouseEvents, true);
	}

	void LineNumberBar::setSeparatorOnRight(bool onRight) {
		SeparatorOnRight = onRight;
		update();
	}

	void LineNumberBar::paintEvent(QPaintEvent* event) {
		Q_UNUSED(event);
		if (Owner == nullptr) {
			return;
		}
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);
		const QPalette& palette = Owner->palette();
		painter.fillRect(rect(), palette.color(QPalette::Base));

		const QList<DiffPane::VisibleLine> lines = Owner->getVisibleLines();
		const QColor fillColor = hatchFillColor(palette);
		const QColor lineColor = hatchLineColor(palette);
		for (const DiffPane::VisibleLine& line : lines) {
			if (line.Number > 0) {
				continue;
			}
			paintDiagonalHatch(painter, QRectF(0, line.Top, width(), line.Height), x(),
				fillColor, lineColor);
		}

		painter.setFont(Owner->getNumberFont());
		QColor numberColor = palette.color(QPalette::Text);
		numberColor.setAlpha(140);
		painter.setPen(numberColor);
		for (const DiffPane::VisibleLine& line : lines) {
			if (line.Number <= 0) {
				continue;
			}
			const QRect target(2, line.Top, qMax(0, width() - 4), line.Height);
			if (target.width() > 0) {
				painter.drawText(target, Qt::AlignRight | Qt::AlignVCenter, QString::number(line.Number));
			}
		}
		painter.setPen(palette.color(QPalette::Mid));
		const int separatorX = SeparatorOnRight ? width() - 1 : 0;
		painter.drawLine(separatorX, 0, separatorX, height());
	}

	void DiffPane::setCodeFont(const QFont& font) {
		CodeFont = font;
		NumberFont = font;
		NumberFont.setPointSizeF(qMax(6.0, font.pointSizeF() * 0.85));
		QTextEdit::setFont(font);
		updateMargin();
		viewport()->update();
		if (Bar) {
			Bar->update();
		}
	}

	QFont DiffPane::getCodeFont() const {
		return CodeFont;
	}

	const QFont& DiffPane::getNumberFont() const {
		return NumberFont;
	}

	DiffPane::LineNumberSide DiffPane::getLineNumberSide() const {
		return Side;
	}

	void DiffPane::setLineNumbers(const QList<qint32>& numbers) {
		LineNumbers = numbers;
		updateMargin();
		if (Bar) {
			Bar->update();
		}
	}

	void DiffPane::setLineNumberSide(LineNumberSide side) {
		Side = side;
		if (Bar) {
			// 行号栏贴在正文一侧的边缘画分隔线。
			Bar->setSeparatorOnRight(side == LineNumberSide::Left);
		}
		updateMargin();
	}

	qint32 DiffPane::numberColumnWidth() const {
		qint32 maxNumber = 0;
		for (qint32 number : LineNumbers) {
			maxNumber = qMax(maxNumber, number);
		}
		qint32 digits = 1;
		while (maxNumber >= 10) {
			maxNumber /= 10;
			++digits;
		}
		const QFontMetrics metrics(NumberFont);
		return metrics.horizontalAdvance(QChar('0')) * digits + 12;
	}

	void DiffPane::updateMargin() {
		MarginWidth = LineNumbers.isEmpty() ? 0 : numberColumnWidth();
		if (Side == LineNumberSide::Left) {
			setViewportMargins(MarginWidth, 0, 0, 0);
		}
		else {
			setViewportMargins(0, 0, MarginWidth, 0);
		}
		updateBarGeometry();
	}

	void DiffPane::updateBarGeometry() {
		if (Bar == nullptr) {
			return;
		}
		if (MarginWidth <= 0) {
			Bar->hide();
			return;
		}
		const QRect viewportRect = viewport()->geometry();
		const QRect barRect = (Side == LineNumberSide::Left)
			? QRect(viewportRect.left() - MarginWidth, viewportRect.top(), MarginWidth, viewportRect.height())
			: QRect(viewportRect.right() + 1, viewportRect.top(), MarginWidth, viewportRect.height());
		Bar->setGeometry(barRect);
		Bar->show();
		Bar->update();
	}

	void DiffPane::resizeEvent(QResizeEvent* event) {
		QTextEdit::resizeEvent(event);
		updateBarGeometry();
	}

	// 占位行（这一侧没有内容）用 45° 斜线阴影铺满整行。
	void DiffPane::paintEvent(QPaintEvent* event) {
		QTextEdit::paintEvent(event);
		if (LineNumbers.isEmpty()) {
			return;
		}
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setClipRect(event->rect());
		const QColor fillColor = hatchFillColor(palette());
		const QColor lineColor = hatchLineColor(palette());

		const qint32 viewportWidth = viewport()->width();
		const qreal originX = viewport()->x();
		for (const VisibleLine& line : getVisibleLines()) {
			if (line.Number > 0 || line.Height <= 0) {
				continue;
			}
			paintDiagonalHatch(painter, QRectF(0, line.Top, viewportWidth, line.Height), originX,
				fillColor, lineColor);
		}
	}

	QList<DiffPane::VisibleLine> DiffPane::getVisibleLines() {
		QList<VisibleLine> result;
		QAbstractTextDocumentLayout* layout = document()->documentLayout();
		if (layout == nullptr) {
			return result;
		}
		const int yOffset = verticalScrollBar()->value();
		const int xOffset = horizontalScrollBar()->value();
		const int viewportHeight = viewport()->height();

		QTextBlock block = document()->findBlock(layout->hitTest(QPointF(xOffset, yOffset + 1), Qt::FuzzyHit));
		if (!block.isValid()) {
			block = document()->firstBlock();
		}
		qint32 number = block.blockNumber();
		while (block.isValid()) {
			const QRectF geometry = layout->blockBoundingRect(block).translated(-xOffset, -yOffset);
			if (geometry.top() > viewportHeight) {
				break;
			}
			if (geometry.bottom() >= 0 && number >= 0 && number < LineNumbers.size()) {
				result.append({ LineNumbers[number], qRound(geometry.top()), qRound(geometry.height()) });
			}
			block = block.next();
			++number;
		}
		return result;
	}

	/*!
		\since YSS 0.18.0
		\a q 所属的比较器。
	*/
	CodeDiffPrivate::CodeDiffPrivate(CodeDiff* q) : q(q) {
		Provider = new BuiltinCodeDiffProvider();
		CodeFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	}

	CodeDiffPrivate::~CodeDiffPrivate() {
		delete Provider;
		Provider = nullptr;
	}

	void CodeDiffPrivate::buildUi() {
		Layout = new QVBoxLayout(q);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(0);

		Splitter = new QSplitter(Qt::Horizontal, q);
		Splitter->setChildrenCollapsible(false);
		Splitter->setHandleWidth(1);

		OldView = new DiffPane(Splitter);
		NewView = new DiffPane(Splitter);
		OldView->setLineNumberSide(DiffPane::LineNumberSide::Right);
		NewView->setLineNumberSide(DiffPane::LineNumberSide::Left);
		// 左侧的滚动条会挡在中缝旁边，两栏共用右侧那一条滚动条。
		OldView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		OldView->setCodeFont(CodeFont);
		NewView->setCodeFont(CodeFont);
		Splitter->addWidget(OldView);
		Splitter->addWidget(NewView);
		Splitter->setStretchFactor(0, 1);
		Splitter->setStretchFactor(1, 1);

		// 两栏块数严格相等，垂直滚动位置直接互相同步。
		QObject::connect(OldView->verticalScrollBar(), &QScrollBar::valueChanged, q, [this](int value) {
			if (SyncingScroll) {
				return;
			}
			SyncingScroll = true;
			NewView->verticalScrollBar()->setValue(value);
			SyncingScroll = false;
			});
		QObject::connect(NewView->verticalScrollBar(), &QScrollBar::valueChanged, q, [this](int value) {
			if (SyncingScroll) {
				return;
			}
			SyncingScroll = true;
			OldView->verticalScrollBar()->setValue(value);
			SyncingScroll = false;
			});

		ConflictBar = new QWidget(q);
		QHBoxLayout* barLayout = new QHBoxLayout(ConflictBar);
		barLayout->setContentsMargins(8, 4, 8, 4);
		barLayout->setSpacing(8);
		ConflictLabel = new QLabel(ConflictBar);
		TakeOldButton = new QPushButton(VITRL("YSS::editor.codeDiff.takeOld"), ConflictBar);
		TakeNewButton = new QPushButton(VITRL("YSS::editor.codeDiff.takeNew"), ConflictBar);
		barLayout->addWidget(ConflictLabel, 1);
		barLayout->addWidget(TakeOldButton);
		barLayout->addWidget(TakeNewButton);
		ConflictBar->hide();

		Layout->addWidget(Splitter, 1);
		Layout->addWidget(ConflictBar);

		QObject::connect(TakeOldButton, &QPushButton::clicked, q, [this]() { choose(false); });
		QObject::connect(TakeNewButton, &QPushButton::clicked, q, [this]() { choose(true); });
	}

	void CodeDiffPrivate::startLoad(const YSSURI& oldUri, const YSSURI& newUri) {
		PendingReads = 2;
		ReadFailed = false;
		ReadError.clear();
		OldData.clear();
		NewData.clear();
		OldView->setPlainText(VITRL("YSS::editor.codeDiff.loading"));
		NewView->clear();

		loadOne(oldUri, true);
		loadOne(newUri, false);
	}

	void CodeDiffPrivate::loadOne(const YSSURI& uri, bool isOldSide) {
		YSSUniqueFileSystem* fs = YSSUniqueFileSystem::getInstance();
		YSSFileSystemReply* reply = fs->read(uri);
		if (reply == nullptr) {
			ReadFailed = true;
			ReadError = VITRL("YSS::editor.codeDiff.readFailed").arg(uri.toString());
			if (--PendingReads == 0) {
				finishLoad();
			}
			return;
		}
		QObject::connect(reply, &YSSFileSystemReply::succeeded, q, [this, reply, isOldSide]() {
			if (isOldSide) {
				OldData = reply->getData();
			}
			else {
				NewData = reply->getData();
			}
			if (--PendingReads == 0) {
				finishLoad();
			}
			});
		QObject::connect(reply, &YSSFileSystemReply::failed, q, [this, uri](const YSSFileError& error) {
			Q_UNUSED(error);
			ReadFailed = true;
			ReadError = VITRL("YSS::editor.codeDiff.readFailed").arg(uri.toString());
			if (--PendingReads == 0) {
				finishLoad();
			}
			});
	}

	void CodeDiffPrivate::finishLoad() {
		if (ReadFailed) {
			OldLines.clear();
			NewLines.clear();
			Ranges.clear();
			Rows.clear();
			resetConflictState();
			OldView->setLineNumbers({});
			NewView->setLineNumbers({});
			OldView->setPlainText(ReadError);
			NewView->clear();
			updateHighlights();
			updateConflictBar();
			return;
		}
		setSource(splitLines(OldData), splitLines(NewData));
	}

	void CodeDiffPrivate::setSource(const QStringList& oldLines, const QStringList& newLines) {
		OldLines = oldLines;
		NewLines = newLines;
		Ranges = Provider->compute(OldLines, NewLines);
		resetConflictState();
		rebuildRows();
		applyRowsToPanes();
		if (ConflictMode) {
			CurrentIndex = Ranges.isEmpty() ? -1 : 0;
		}
		updateHighlights();
		updateConflictBar();
		if (ConflictMode) {
			// 控件可能尚未完成布局，滚动到当前差异延迟到下一轮事件循环。
			QTimer::singleShot(0, q, [this]() { focusCurrentRange(); });
		}
	}

	// 把相异范围展开成逐行显示表：差异块内左右按序号配对，多出来的一侧用空行占位。
	void CodeDiffPrivate::rebuildRows() {
		Rows.clear();
		qint32 oldPos = 0;
		qint32 newPos = 0;
		for (qint32 r = 0; r < Ranges.size(); ++r) {
			const qint32 oldStart = std::get<0>(Ranges[r]);
			const qint32 oldSize = std::get<1>(Ranges[r]);
			const qint32 newStart = std::get<2>(Ranges[r]);
			const qint32 newSize = std::get<3>(Ranges[r]);
			while (oldPos < oldStart && oldPos < OldLines.size()) {
				DiffRow row;
				row.OldNumber = oldPos + 1;
				row.NewNumber = newPos + 1;
				row.OldText = OldLines.value(oldPos);
				row.NewText = NewLines.value(newPos);
				Rows.append(row);
				++oldPos;
				++newPos;
			}
			const qint32 pairCount = qMax(oldSize, newSize);
			for (qint32 k = 0; k < pairCount; ++k) {
				DiffRow row;
				row.RangeIndex = r;
				if (k < oldSize) {
					row.OldNumber = oldStart + k + 1;
					row.OldText = OldLines.value(oldStart + k);
				}
				if (k < newSize) {
					row.NewNumber = newStart + k + 1;
					row.NewText = NewLines.value(newStart + k);
				}
				Rows.append(row);
			}
			oldPos = oldStart + oldSize;
			newPos = newStart + newSize;
		}
		while (oldPos < OldLines.size() || newPos < NewLines.size()) {
			DiffRow row;
			if (oldPos < OldLines.size()) {
				row.OldNumber = oldPos + 1;
				row.OldText = OldLines.value(oldPos);
				++oldPos;
			}
			if (newPos < NewLines.size()) {
				row.NewNumber = newPos + 1;
				row.NewText = NewLines.value(newPos);
				++newPos;
			}
			Rows.append(row);
		}
	}

	void CodeDiffPrivate::applyRowsToPanes() {
		QStringList oldTexts;
		QStringList newTexts;
		QList<qint32> oldNumbers;
		QList<qint32> newNumbers;
		oldTexts.reserve(Rows.size());
		newTexts.reserve(Rows.size());
		oldNumbers.reserve(Rows.size());
		newNumbers.reserve(Rows.size());
		for (const DiffRow& row : Rows) {
			oldTexts.append(row.OldText);
			newTexts.append(row.NewText);
			oldNumbers.append(row.OldNumber);
			newNumbers.append(row.NewNumber);
		}
		OldView->setPlainText(oldTexts.join(QChar('\n')));
		NewView->setPlainText(newTexts.join(QChar('\n')));
		OldView->setLineNumbers(oldNumbers);
		NewView->setLineNumbers(newNumbers);
	}

	// 把 [start, start+size) 这些行整行选中，用于绘制行背景。
	// 空行没有可选的文本，靠 FullWidthSelection 让它也有背景。
	static void appendRowSelection(QList<QTextEdit::ExtraSelection>& out, DiffPane* view,
		qint32 rowIndex, const QColor& color) {
		if (view == nullptr) {
			return;
		}
		const QTextBlock block = view->document()->findBlockByNumber(rowIndex);
		if (!block.isValid()) {
			return;
		}
		QTextCursor cursor(view->document());
		cursor.setPosition(block.position());
		cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
		QTextEdit::ExtraSelection selection;
		selection.cursor = cursor;
		selection.format.setBackground(color);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		out.append(selection);
	}

	static QColor diffColor(bool current, bool isOldSide) {
		const int alpha = current ? 130 : 70;
		return isOldSide ? QColor(220, 80, 80, alpha) : QColor(80, 180, 100, alpha);
	}

	static QStringList splitLines(const QByteArray& data) {
		QString text = QString::fromUtf8(data);
		if (text.startsWith(QChar(0xFEFF))) {
			text.remove(0, 1);
		}
		text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
		text.replace(QChar('\r'), QChar('\n'));
		QStringList lines = text.split(QChar('\n'));
		// 文件末尾的换行不构成额外的一行。
		if (!lines.isEmpty() && lines.last().isEmpty()) {
			lines.removeLast();
		}
		return lines;
	}

	void CodeDiffPrivate::resetConflictState() {
		Selections = QList<bool>(Ranges.size(), false);
		Resolved = QList<bool>(Ranges.size(), false);
		CurrentIndex = -1;
	}

	void CodeDiffPrivate::updateHighlights() {
		QList<QTextEdit::ExtraSelection> oldSelections;
		QList<QTextEdit::ExtraSelection> newSelections;
		for (qint32 i = 0; i < Rows.size(); ++i) {
			const DiffRow& row = Rows[i];
			if (row.RangeIndex < 0) {
				continue;
			}
			const bool current = ConflictMode && row.RangeIndex == CurrentIndex;
			if (row.OldNumber > 0) {
				appendRowSelection(oldSelections, OldView, i, diffColor(current, true));
			}
			if (row.NewNumber > 0) {
				appendRowSelection(newSelections, NewView, i, diffColor(current, false));
			}
		}
		OldView->setExtraSelections(oldSelections);
		NewView->setExtraSelections(newSelections);
	}

	void CodeDiffPrivate::updateConflictBar() {
		if (!ConflictMode) {
			ConflictBar->hide();
			return;
		}
		ConflictBar->show();
		const qint32 total = Ranges.size();
		qint32 resolved = 0;
		for (bool item : Resolved) {
			if (item) {
				++resolved;
			}
		}
		if (total == 0) {
			ConflictLabel->setText(VITRL("YSS::editor.codeDiff.noDifference"));
			TakeOldButton->setEnabled(false);
			TakeNewButton->setEnabled(false);
			return;
		}
		const bool done = resolved >= total;
		if (done) {
			ConflictLabel->setText(VITRL("YSS::editor.codeDiff.allResolved").arg(total));
		}
		else if (CurrentIndex >= 0 && CurrentIndex < total) {
			ConflictLabel->setText(VITRL("YSS::editor.codeDiff.progress")
				.arg(CurrentIndex + 1).arg(total).arg(resolved));
		}
		else {
			ConflictLabel->setText(VITRL("YSS::editor.codeDiff.progressSimple")
				.arg(resolved).arg(total));
		}
		TakeOldButton->setEnabled(!done);
		TakeNewButton->setEnabled(!done);
	}

	qint32 CodeDiffPrivate::firstRowOfRange(qint32 rangeIndex) const {
		for (qint32 i = 0; i < Rows.size(); ++i) {
			if (Rows[i].RangeIndex == rangeIndex) {
				return i;
			}
		}
		return -1;
	}

	void CodeDiffPrivate::focusCurrentRange() {
		const qint32 row = firstRowOfRange(CurrentIndex);
		if (row < 0) {
			return;
		}
		const int target = qMax(0, row - 2);
		NewView->verticalScrollBar()->setValue(target);
		OldView->verticalScrollBar()->setValue(target);
	}

	void CodeDiffPrivate::choose(bool takeNew) {
		const qint32 total = Ranges.size();
		if (!ConflictMode || CurrentIndex < 0 || CurrentIndex >= total) {
			return;
		}
		Selections[CurrentIndex] = takeNew;
		Resolved[CurrentIndex] = true;
		qint32 next = -1;
		for (qint32 step = 1; step <= total; ++step) {
			const qint32 candidate = (CurrentIndex + step) % total;
			if (!Resolved[candidate]) {
				next = candidate;
				break;
			}
		}
		CurrentIndex = next;
		updateHighlights();
		updateConflictBar();
		if (CurrentIndex >= 0) {
			focusCurrentRange();
		}
	}

	void CodeDiffPrivate::setCodeFont(const QFont& font) {
		CodeFont = font;
		if (OldView) {
			OldView->setCodeFont(font);
		}
		if (NewView) {
			NewView->setCodeFont(font);
		}
	}

	QStringList CodeDiffPrivate::mergedLines() const {
		QStringList result;
		qint32 oldPos = 0;
		for (qint32 i = 0; i < Ranges.size(); ++i) {
			const qint32 oldStart = std::get<0>(Ranges[i]);
			const qint32 oldSize = std::get<1>(Ranges[i]);
			const qint32 newStart = std::get<2>(Ranges[i]);
			const qint32 newSize = std::get<3>(Ranges[i]);
			for (qint32 j = oldPos; j < oldStart && j < OldLines.size(); ++j) {
				result.append(OldLines[j]);
			}
			const bool takeNew = i < Selections.size() ? Selections[i] : false;
			if (takeNew) {
				for (qint32 j = newStart; j < newStart + newSize && j < NewLines.size(); ++j) {
					result.append(NewLines[j]);
				}
			}
			else {
				for (qint32 j = oldStart; j < oldStart + oldSize && j < OldLines.size(); ++j) {
					result.append(OldLines[j]);
				}
			}
			oldPos = oldStart + oldSize;
		}
		for (qint32 j = oldPos; j < OldLines.size(); ++j) {
			result.append(OldLines[j]);
		}
		return result;
	}

	/*!
		\since YSS 0.18.0
		\a parent 父控件。
		构造比较器控件。默认使用内置差异分析器，左右并列、只读显示。
	*/
	CodeDiff::CodeDiff(QWidget* parent) : FileEditWidget(parent) {
		d = new CodeDiffPrivate(this);
		setAutoAbandon(true);
		d->buildUi();
	}

	CodeDiff::~CodeDiff() {
		delete d;
		d = nullptr;
	}

	/*!
		\since YSS 0.18.0
		\a oldLines 旧侧的文本行。
		\a newLines 新侧的文本行。

		设置比较源。会重新计算相异范围并清空已有的取舍结果。
	*/
	void CodeDiff::setDiffSource(const QStringList& oldLines, const QStringList& newLines) {
		d->setSource(oldLines, newLines);
	}

	/*!
		\since YSS 0.18.0
		return 相异范围列表，元素为 (旧侧起始行, 旧侧行数, 新侧起始行, 新侧行数)。
	*/
	QList<CodeDiff::DiffRange> CodeDiff::getDiffRanges() const {
		return d->Ranges;
	}

	/*!
		\since YSS 0.18.0
		\a enable 是否进入取舍状态。

		进入后显示取舍栏，并从第一处差异开始依次询问；退出后只保留差异高亮。
	*/
	void CodeDiff::setConflictModeEnable(bool enable) {
		if (d->ConflictMode == enable) {
			return;
		}
		d->ConflictMode = enable;
		d->CurrentIndex = (enable && !d->Ranges.isEmpty()) ? 0 : -1;
		d->updateHighlights();
		d->updateConflictBar();
		if (enable) {
			d->focusCurrentRange();
		}
	}

	/*!
		\since YSS 0.18.0
		return 当前是否处于取舍状态。
	*/
	bool CodeDiff::isConflictModeEnabled() const {
		return d->ConflictMode;
	}

	/*!
		\since YSS 0.18.0
		return 每一处差异的取舍结果，与 \l{getDiffRanges()} 一一对应。
		true 表示取右侧（新），false 表示取左侧（旧）。尚未询问的差异保持 false。
	*/
	QList<bool> CodeDiff::getConflictSelection() const {
		return d->Selections;
	}

	/*!
		\since YSS 0.18.0
		return 按当前取舍结果合并后的全文（按行）。
		尚未取舍的差异按"取左侧"处理。
	*/
	QStringList CodeDiff::getConflictMerged() const {
		return d->mergedLines();
	}

	/*!
		\since YSS 0.18.0
		\a font 两侧文本使用的字体。行号使用同族字体的 85% 字号。
	*/
	void CodeDiff::setTextFont(const QFont& font) {
		QFont target = font;
		if (d->CodeFont.pointSizeF() > 0 && target.pointSizeF() <= 0) {
			target.setPointSizeF(d->CodeFont.pointSizeF());
		}
		d->setCodeFont(target);
	}

	/*!
		\since YSS 0.18.0
		return 两侧文本当前使用的字体。
	*/
	QFont CodeDiff::getTextFont() const {
		return d->CodeFont;
	}

	/*!
		\since YSS 0.18.0
		\a size 字号（磅）。只改字号，不改字体族。
	*/
	void CodeDiff::setFontSize(qint32 size) {
		if (size <= 0) {
			return;
		}
		QFont target = d->CodeFont;
		target.setPointSize(size);
		d->setCodeFont(target);
	}

	/*!
		\since YSS 0.18.0
		return 当前字号（磅）。
	*/
	qint32 CodeDiff::getFontSize() const {
		return d->CodeFont.pointSize();
	}

	/*!
		\since YSS 0.18.0
		\a ext 虚拟文件扩展名。
		\a fileName 显示用文件名。
		\a param JSON 参数，需要包含 \c old_uri 与 \c new_uri 两个 YSSURI 字符串。
		return 是否受理。

		读取是异步的，本函数返回 true 只表示参数合法并已提交读取。
	*/
	bool CodeDiff::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		Q_UNUSED(fileName);
		if (ext != QStringLiteral(YSSVirtualFile_CodeDiff)) {
			return false;
		}
		Visindigo::Utility::JsonConfig config(param);
		bool oldOk = false;
		bool newOk = false;
		const QString oldUriText = config.getValue(QStringLiteral("old_uri"), &oldOk).toString();
		const QString newUriText = config.getValue(QStringLiteral("new_uri"), &newOk).toString();
		if (!oldOk || !newOk || oldUriText.isEmpty() || newUriText.isEmpty()) {
			return false;
		}
		const YSSURI oldUri = YSSURI::fromString(oldUriText);
		const YSSURI newUri = YSSURI::fromString(newUriText);
		if (!oldUri.isValid() || !newUri.isValid()) {
			return false;
		}
		d->startLoad(oldUri, newUri);
		return true;
	}

	/*!
		\since YSS 0.18.0
		return 是否可以关闭。比较器没有需要保存的内容，总是返回 true。
	*/
	bool CodeDiff::onVirtualClose() {
		d->OldLines.clear();
		d->NewLines.clear();
		d->Ranges.clear();
		d->Rows.clear();
		d->resetConflictState();
		return true;
	}
}
