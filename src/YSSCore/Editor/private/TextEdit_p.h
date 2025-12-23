#pragma once
#include <QtCore/qobject.h>
#include <QtGui/qfont.h>
#include <QtGui/qtextcursor.h>
#include <QtCore/qpoint.h>
#include <QtGui/qevent.h>
// Forward declarations
namespace YSSCore::Editor {
	class TextEdit;
	class TabCompleterProvider;
	class HoverInfoProvider;
}
class QTextEdit;
class QHBoxLayout;
class QFontMetricsF;
class QSyntaxHighlighter;
class QKeyEvent;
class QMouseEvent;
namespace YSSCore::__Private__ {
	class TabCompleterWidget;
	class HoverInfoWidget;
}
// Main Implementation
namespace YSSCore::__Private__ {
	class TextEditPrivate :public QObject {
		Q_OBJECT;
		friend class YSSCore::Editor::TextEdit;
		friend class YSSCore::Editor::HoverInfoProvider;
	protected:
		QTextEdit* Line = nullptr;
		QTextEdit* Text = nullptr;
		QHBoxLayout* Layout = nullptr;
		qint32 LineCount = 0;
		qint8 TabWidth = 4;
		qint32 LastCursorLine = 0;
		QTextCursor LastCursor;
		QFont Font;
		QFontMetricsF* FontMetrics;
		QSyntaxHighlighter* Highlighter = nullptr;
		YSSCore::Editor::TabCompleterProvider* TabCompleter = nullptr;
		YSSCore::__Private__::TabCompleterWidget* TabCompleterWidget = nullptr;
		YSSCore::Editor::HoverInfoProvider* HoverInfoProvider = nullptr;
		YSSCore::__Private__::HoverInfoWidget* HoverInfoWidget = nullptr;
		bool ReloadTab = true;
		qint32 HoverTimeout = 800;
		QTimer* HoverTimer = nullptr;
		QPoint LastMousePos;
		TextEditPrivate() {};
		~TextEditPrivate();
	protected:
		void onBlockCountChanged(qint32 count);
		void onTabClicked(QKeyEvent* event);
		void onTabClicked_TabCompleter(QKeyEvent* event);
		void onTabClicked_NormalInput(QKeyEvent* event);
		void onEnterClicked(QKeyEvent* event);
		void onDirectionClicked(QKeyEvent* event);
		void onMouseMove(QMouseEvent* event);
		bool onMouseScroll(QWheelEvent* event);
		void onHoverTimeout();
		void onHoverInfo(bool triggeFromHover);
		void onCursorPositionChanged();
		void onCompleter();
		void onScrollBarChanged(int value);
	};
}