#pragma once
#include <QtCore/qobject.h>
#include <QtWidgets/qtextbrowser.h>
// Forward declarations
namespace YSSCore::Editor {
	class TextEdit;
	class TabCompleterProvider;
}
class QTextEdit;
class QHBoxLayout;
class QTextCursor;
class QFontMetricsF;
class QSyntaxHighlighter;
class HoverTip;
namespace YSSCore::__Private__ {
	class TabCompleterWidget;
}
// Main Implementation
namespace YSSCore::__Private__ {
	class TextEditPrivate :public QObject {
		Q_OBJECT;
		friend class YSSCore::Editor::TextEdit;
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
		bool ReloadTab = true;
		HoverTip* Hover = nullptr;
		TextEditPrivate() {};
		~TextEditPrivate();
	protected:
		void onBlockCountChanged(qint32 count);
		void onTabClicked(QKeyEvent* event);
		void onEnterClicked(QKeyEvent* event);
		void onMouseMove(QMouseEvent* event);
		void onCursorPositionChanged();
		void onCompleter();
	};
}