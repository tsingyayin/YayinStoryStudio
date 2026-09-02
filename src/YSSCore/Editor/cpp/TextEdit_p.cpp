#include "Editor/private/TextEdit_p.h"
#include "Editor/TextEdit.h"
#include "Editor/DocumentMessage.h"
#include "Editor/DocumentMessageManager.h"
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlineedit.h>
#include <General/TranslationHost.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtGui/qpainter.h>
#include <QtGui/qtextlayout.h>
#include <QtGui/qtextobject.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qstyleoption.h>
#include <QtCore/qtimer.h>

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
		connect(YSSCore::Editor::DocumentMessageManager::getInstance(), &YSSCore::Editor::DocumentMessageManager::messageChanged,
			this, &DocumentOverviewLabel::onMessageChanged);
		connect(YSSCore::Editor::DocumentMessageManager::getInstance(), &YSSCore::Editor::DocumentMessageManager::messageChangedForLine,
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

	void DocumentOverviewLabel::onMessageChanged(const QString& filePath) {
		if (this->editor && this->editor->getFilePath() == filePath) {
			rebuildErrorLines();
		}
	}

	void DocumentOverviewLabel::onMessageChangedForLine(const QString& filePath, qint32 lineNumber) {
		if (!this->editor || this->editor->getFilePath() != filePath) return;

		auto messages = YSSCore::Editor::DocumentMessageManager::getInstance()->getMessages(filePath, lineNumber);
		if (!messages.isEmpty()) {
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

		auto allMessages = YSSCore::Editor::DocumentMessageManager::getInstance()->getAllMessages(this->editor->getFilePath());
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
}