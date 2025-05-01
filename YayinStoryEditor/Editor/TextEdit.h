#pragma once
#include <QtWidgets>

namespace YSE::Editor {
	class HoverTip;

	class TextEdit :public QWidget {
		Q_OBJECT;
	private:
		QTextEdit* Line;
		QTextEdit* Text;
		QHBoxLayout* Layout;
		qint32 LineCount = 0;
		qint8 TabWidth = 4;
		QFont Font;
		QFontMetricsF* FontMetrics;
		QSyntaxHighlighter* Highlighter;
		bool ReloadTab = true;
		HoverTip* Hover = nullptr;
	public:
		TextEdit(QWidget* parent = nullptr);
	private:
		void onBlockCountChanged(qint32 count);
		virtual bool eventFilter(QObject* obj, QEvent* event) override;
		void onTabClicked(QKeyEvent* event);
		void onEnterClicked(QKeyEvent* event);
		void onMouseMove(QMouseEvent* event);
	};
}
