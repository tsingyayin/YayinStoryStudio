#ifndef YSSCore_Editor_private_TextEdit_p_h
#define YSSCore_Editor_private_TextEdit_p_h
#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qfont.h>
#include <QtGui/qtextcursor.h>
#include <QtCore/qpoint.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qtextedit.h>
#include <Widgets/BorderFrame.h>
#include <Widgets/ThemeManager.h>
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
class QToolButton;
class QCheckBox;
class QScrollBar;
namespace YSSCore::__Private__ {
	class TabCompleterWidget;
	class HoverInfoWidget;
	class SuggestionWidget;

	/*!
		建议的呈现方式。
		\list
		\li Inline 行内建议：只作用于目标行，不增加行。pureAdd 时把文字追加到行尾，否则替换整行。
		\li Block 多行建议：在目标行之前插入若干行，因此需要在行间画出插入位置指示。
		\li Remove 行删除建议：删除从目标行开始的若干行，在要删除的行上叠一层半透明红底。
		\endlist
	*/
	enum class SuggestionKind { None = 0, Inline, Block, Remove };
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

	/*!
		行号栏。

		旧实现用 QTextBrowser 承载行号，每新增一行就要 append 一次并顺带
		setBlockFormat —— 打开几万行的文件时光这一段就要几秒（实测 37706 行约 4.7 秒）。
		行号本身只是"第几行"这几个字符，没必要真的建 37706 个文本块，因此这里改成
		自绘：布局由 QTextDocument 负责，本控件只按文档坐标取可见块的矩形，画出对应的行号。
	*/
	class LineNumberBar : public QWidget {
	public:
		LineNumberBar(QTextEdit* editor, QWidget* parent = nullptr);
		void updateLineCount(qint32 count);
		void refreshMetrics();
	protected:
		void paintEvent(QPaintEvent* event) override;
		void wheelEvent(QWheelEvent* event) override;
	private:
		QTextEdit* editor = nullptr;
		qint32 lineCount = 1;
		qint32 cachedDigits = 1;
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
		QList<QColor> colorsOf(const QTextBlock& block) const;
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
		void paintSuggestionIndicator(QPainter& painter, const QRect& clipRect);
		void paintPureAddSuggestion(QPainter& painter, const QRect& clipRect);
		void paintRemoveSuggestion(QPainter& painter, const QRect& clipRect);
		bool visibleLines(qint32& firstLine, qint32& lastLine, const QRect& clipRect) const;
	};

	/*!
		待抉择建议的浮窗。三种建议共用这一个控件：
		\list
		\li 行内建议：显示序号与建议文字（pureAdd 时文字也画在行尾，这里仍保留一份便于行外查看）。
		\li 多行建议：显示序号与要插入的若干行。
		\li 行删除建议：没有序号和正文，只有标题与接受/拒绝。
		\endlist

		顶部一行显示序号、小标题（行内/多行/行删除建议）和接受(√)、拒绝(×)按钮。
		目标行不在视口内时，小标题会变成"向上/向下滚动到行 xxx"，并在其后显示一个同色箭头。
		它不抢焦点：接受与拒绝同时支持键盘（Tab/Enter 接受，Esc 拒绝），按钮只是鼠标的等价入口。
	*/
	class SuggestionWidget :public Visindigo::Widgets::BorderFrame, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
		friend class YSSCore::__Private__::TextEditPrivate;
		friend class YSSCore::Editor::TextEdit;
	signals:
		void accepted();
		void rejected();
	public:
		SuggestionWidget(YSSCore::Editor::TextEdit* editor, QWidget* parent = nullptr);
		void setIndex(qint32 index, qint32 total);
		void setTitle(const QString& title);
		void setScrollArrow(qint32 direction);
		void setContent(const QString& content);
		void applySize();
	protected:
		YSSCore::Editor::TextEdit* editor = nullptr;
		QVBoxLayout* layout = nullptr;
		QHBoxLayout* titleLayout = nullptr;
		QLabel* indexLabel = nullptr;
		QLabel* titleLabel = nullptr;
		QLabel* arrowLabel = nullptr;
		QLabel* contentLabel = nullptr;
		QToolButton* acceptButton = nullptr;
		QToolButton* rejectButton = nullptr;
		QIcon scrollUpIcon;
		QIcon scrollDownIcon;
		virtual void onThemeChanged() override;
	};

	class TextEditPrivate :public QObject {
		Q_OBJECT;
		friend class DocumentOverviewLabel;
		friend class InlayHintTextEdit;
		friend class YSSCore::Editor::TextEdit;
		friend class YSSCore::Editor::HoverInfoProvider;
		friend class YSSCore::Editor::SyntaxHighlighter;
	protected:
		YSSCore::Editor::TextEdit* q = nullptr;
		LineNumberBar* Line = nullptr;
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
		// 待抉择建议：SgtLine 在 Inline/Remove 时是目标行，在 Block 时是要在其之前插入的行号。
		// SgtItems 中每一项都是一条建议，行内建议只有一行，多行建议有多行；删除建议没有候选内容。
		SuggestionKind SgtKind = SuggestionKind::None;
		qint32 SgtLine = -1;
		QList<QStringList> SgtItems;
		qint32 SgtIndex = 0;
		qint32 SgtRemoveSize = 0;
		bool SgtPureAdd = false;
		QString SgtTitleKey;
		bool ApplyingSuggestion = false;
		YSSCore::__Private__::SuggestionWidget* SuggestionWidget = nullptr;
		// 临时测试入口：编辑区右上角的小按钮，用来手动触发上面这套建议 UI。
		QPushButton* TestButton = nullptr;
		qint32 TestDemoIndex = 0;
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
		void showSuggestion(SuggestionKind kind, qint32 line, const QList<QStringList>& items, bool pureAdd = false);
		void showRemoveSuggestion(qint32 from, qint32 size);
		void dismissSuggestion(bool notifyRejected);
		void acceptSuggestion();
		void cycleSuggestion(qint32 delta);
		void updateSuggestionWidget();
		void updateSuggestionTitle();
		QString currentSuggestionText() const;
		bool isSuggestionLineVisible(bool& above) const;
		void adjustSuggestionPosition();
		void positionTestButton();
		void runSuggestionDemo();
	};
}
#endif // YSSCore_Editor_private_TextEdit_p_h
