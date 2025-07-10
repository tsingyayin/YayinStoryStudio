#pragma once
#include <QtCore/qobject.h>
#include <QtWidgets/qtextbrowser.h>
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
		QSyntaxHighlighter* Highlighter;
		YSSCore::Editor::TabCompleterProvider* TabCompleter = nullptr;
		bool ReloadTab = true;
		HoverTip* Hover = nullptr;
		TextEditPrivate() {};
		~TextEditPrivate() {
			if (FontMetrics != nullptr) {
				delete FontMetrics;
			}
			// do not delete Highlighter, it will be automaticly deleted by QTextDocument.
		}
	protected:
		void onBlockCountChanged(qint32 count);
		void onTabClicked(QKeyEvent* event);
		void onEnterClicked(QKeyEvent* event);
		void onMouseMove(QMouseEvent* event);
		void onCursorPositionChanged();
	};
}