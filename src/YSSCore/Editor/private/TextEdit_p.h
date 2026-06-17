#pragma once
#include <QtCore/qobject.h>
#include <QtGui/qfont.h>
#include <QtGui/qtextcursor.h>
#include <QtCore/qpoint.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qtextedit.h>
#include <Widgets/BorderFrame.h>
#include <QtGui/qabstracttextdocumentlayout.h>
#include "Editor/TabCompleterProvider.h"

// Forward declarations
namespace YSSCore::Editor {
	class TextEdit;
	class TabCompleterProvider;
	class HoverInfoProvider;
	class SyntaxHighlighter;
}
class QVBoxLayout;
class QComboBox;
class QTextEdit;
class QHBoxLayout;
class QFontMetricsF;
class QSyntaxHighlighter;
class QKeyEvent;
class QMouseEvent;
class QGridLayout;
class QLineEdit;
class QLabel;
class QPushButton;
class QCheckBox;
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

	class TextEditPrivate :public QObject {
		Q_OBJECT;
		friend class YSSCore::Editor::TextEdit;
		friend class YSSCore::Editor::HoverInfoProvider;
		friend class YSSCore::Editor::SyntaxHighlighter;
	protected:
		YSSCore::Editor::TextEdit* q = nullptr;
		QTextEdit* Line = nullptr;
		QTextEdit* Text = nullptr;
		QGridLayout* Layout = nullptr;
		qint32 LineCount = 0;
		qint8 TabWidth = 4;
		bool ReloadTab = false;
		bool useKeyboardToMoveCursor = false;
		bool Rehighlighting = false;
		QFontMetricsF* FontMetrics = nullptr;
		YSSCore::Editor::SyntaxHighlighter* Highlighter = nullptr;
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