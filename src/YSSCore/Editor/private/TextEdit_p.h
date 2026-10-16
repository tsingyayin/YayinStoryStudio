#ifndef YSSCore_Editor_private_TextEdit_p_h
#define YSSCore_Editor_private_TextEdit_p_h
#include <QtCore/qobject.h>
#include <QtGui/qfont.h>
#include <QtGui/qtextcursor.h>
#include <QtCore/qpoint.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qtextedit.h>
#include <Widgets/BorderFrame.h>
#include <QtGui/qabstracttextdocumentlayout.h>
#include "Editor/TabCompleterProvider.h"
#include <QtGui/qimage.h>
#include <QtWidgets/qlabel.h>
// Forward declarations
namespace YSSCore::Editor {
	class TextEdit;
	class TabCompleterProvider;
	class HoverInfoProvider;
	class SyntaxHighlighter;
	class LangServer;
	class InlayHint;
}
class QVBoxLayout;
class QComboBox;
class QTextEdit;
class QHBoxLayout;
class QFontMetricsF;
class QPainter;
class QSyntaxHighlighter;
class QKeyEvent;
class QMouseEvent;
class QGridLayout;
class QLineEdit;
class QLabel;
class QPushButton;
class QCheckBox;
class QScrollBar;
namespace YSSCore::__Private__ {
	class TabCompleterWidget;
	class HoverInfoWidget;
}
// Main Implementation
namespace YSSCore::__Private__ {
	class TextEditFindAndReplace :public Visindigo::Widgets::BorderFrame {
		Q_OBJECT;
		friend class YSSCore::Editor::TextEdit;
		friend class TextEditPrivate;
	protected:
		TextEditFindAndReplace(YSSCore::Editor::TextEdit* parent = nullptr);
		~TextEditFindAndReplace();
		void setFindText(const QString& text);
		void findAll();
	protected:
		YSSCore::Editor::TextEdit* parent;
		QGridLayout* layout;
		QLabel* titleLabel;
		QPushButton* closeButton;
		QCheckBox* sourceAsReCheckBox;
		QCheckBox* caseSensitiveCheckBox;
		QCheckBox* wholeWordCheckBox;
		QLabel* rawInputLabel;
		QLineEdit* rawInput;
		QLabel* replaceInputLabel;
		QLineEdit* replaceInput;
		QLabel* matchCountLabel;
		QPushButton* findNextButton;
		QPushButton* replaceNextButton;
		QPushButton* replaceAllButton;
	};

	class ViewportIndicator : public QWidget {
	public:
		using QWidget::QWidget;
	protected:
		void paintEvent(QPaintEvent* event) override;
	};

	class DocumentOverviewLabel : public QWidget {
		Q_OBJECT;
		friend class YSSCore::Editor::TextEdit;
		friend class TextEditPrivate;
	public:
		DocumentOverviewLabel(YSSCore::Editor::TextEdit* editor, QWidget* parent = nullptr);
	protected:
		void paintEvent(QPaintEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		bool eventFilter(QObject* obj, QEvent* event) override;
		QSize sizeHint() const override;
	private:
		YSSCore::Editor::TextEdit* editor = nullptr;
		QTextDocument* document = nullptr;
		QList<QList<QColor>> lineColors;
		QList<int> errorLines;

		ViewportIndicator* viewportIndicator = nullptr;
		bool draggingViewport = false;
		int dragStartY = 0;
		int dragStartScrollValue = 0;

		void onContentsChange(int position, int charsRemoved, int charsAdded);
		void onMessageChanged();
		void onMessageChangedForLine(qint32 lineNumber);
		void recalculateAll();
		void recalculateBlock(int blockNumber);
		void rebuildErrorLines();
		void updateViewportIndicator();
	};

	/*!
		文本编辑区的实际控件。重写paintEvent是为了在文本之上绘制内联提示：
		QTextEdit::paintEvent里创建的QPainter作用于viewport，因此这里同样作用于viewport，
		并直接使用光标矩形（也是viewport坐标）定位提示，不需要自己做滚动偏移换算。

		\note 重写paintEvent时创建QPainter必须作用于viewport，这是QTextEdit的要求。
	*/
	class InlayHintTextEdit :public QTextEdit {
		Q_OBJECT;
	public:
		InlayHintTextEdit(YSSCore::Editor::TextEdit* editor, QWidget* parent = nullptr);
	protected:
		virtual void paintEvent(QPaintEvent* event) override;
	public slots:
		void onInlayHintChangedForLine(qint32 lineNumber);
	public:
		void refreshInlayHintSpacings();
	private:
		YSSCore::Editor::TextEdit* editor = nullptr;
		void paintInlayHints(QPainter& painter, const QRect& clipRect);
		void paintInlayHint(QPainter& painter, const QRectF& gapRect, const YSSCore::Editor::InlayHint& hint) const;
		bool visibleLines(qint32& firstLine, qint32& lastLine, const QRect& clipRect) const;
	};

	class TextEditPrivate :public QObject {
		Q_OBJECT;
		friend class DocumentOverviewLabel;
		friend class YSSCore::Editor::TextEdit;
		friend class YSSCore::Editor::HoverInfoProvider;
		friend class YSSCore::Editor::SyntaxHighlighter;
	protected:
		YSSCore::Editor::TextEdit* q = nullptr;
		QTextEdit* Line = nullptr;
		InlayHintTextEdit* Text = nullptr;
		DocumentOverviewLabel* Overview = nullptr;
		QScrollBar* HScrollBar = nullptr;
		QGridLayout* Layout = nullptr;
		qint32 LineCount = 0;
		qint8 TabWidth = 4;
		bool ReloadTab = false;
		bool useKeyboardToMoveCursor = false;
		bool Rehighlighting = false;
		// 打开文件时探测到的编码与换行符，保存时按原样写回：否则会把 UTF-16 文件写成
		// UTF-8、或者把 CRLF 文档悄悄改成 LF。Encoding 为 Unsupported 表示这种编码
		// 无法安全改写（例如 GBK），此时会拒绝保存，以免把原始内容变成乱码。
		enum class TextEncoding { Utf8 = 0, Utf8Bom, Utf16LE, Utf16BE, Unsupported };
		TextEncoding Encoding = TextEncoding::Utf8;
		bool UseCRLF = false;
		qreal InlayHintFontScale = 0.6;
		bool InlayHintVisible = true;
		QFontMetricsF* FontMetrics = nullptr;
		YSSCore::Editor::SyntaxHighlighter* Highlighter = nullptr;
		YSSCore::Editor::LangServer* LangServer = nullptr;
		YSSCore::Editor::TabCompleterProvider* TabCompleter = nullptr;
		YSSCore::__Private__::TabCompleterWidget* TabCompleterWidget = nullptr;
		YSSCore::Editor::HoverInfoProvider* HoverInfoProvider = nullptr;
		YSSCore::__Private__::HoverInfoWidget* HoverInfoWidget = nullptr;
		qint32 HoverTimeout = 400;
		YSSCore::Editor::TabCompleterItem::CompleterLevel CompleterLevel = YSSCore::Editor::TabCompleterItem::CompleterLevel::All;
		QTimer* HoverTimer = nullptr;
		QPoint LastMousePos;
		QWidget* HoverArea = nullptr;
		QList<QTextEdit::ExtraSelection> AltMultiSelections;
		QList<QTextEdit::ExtraSelection> FindAllMultiSelections;
		QTextEdit::ExtraSelection CurrentLineSelection;
		TextEditFindAndReplace* FindAndReplaceWidget = nullptr;
		YSSCore::Editor::TabCompleterItem::ItemTypes CompleterTypeFilter = YSSCore::Editor::TabCompleterItem::ItemType::AllType;
		TextEditPrivate() {};
		~TextEditPrivate();
	public:
		bool eventFilter(QObject* watched, QEvent* event) override;
	protected:
		void onBlockCountChanged(qint32 count);
		void onTabClicked(QKeyEvent* event);
		void onTabClicked_TabCompleter(QKeyEvent* event);
		void onTabClicked_NormalInput(QKeyEvent* event);
		void onEnterClicked(QKeyEvent* event);
		void onDirectionClicked(QKeyEvent* event);
		void onMouseMove(QMouseEvent* event);
		bool onMouseScroll(QWheelEvent* event);
		void onEscapeClicked(QKeyEvent* event);
		void onHoverTimeout();
		void onHoverInfo(bool triggeFromHover);
		void onCursorPositionChanged();
		void onCompleter();
		void onScrollBarChanged(int value);
		void onAltMultiSelection(QKeyEvent* event);
		void onAltSwapLine(QKeyEvent* event);
		void onDoubleLine(QKeyEvent* event);
		void clearAltMultiSelection();
		void createFindAllMultiSelection(QList<QTextCursor> findResults);
		void clearFindAllMultiSelection();
		void showFindAndReplace();
		void onMultiSelectionChanged();
		void adjustTabCompleterPosition();
		void adjustHoverInfoPosition(const QTextCursor& cursor);
	};
}
#endif // YSSCore_Editor_private_TextEdit_p_h
