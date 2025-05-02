#pragma once
#include <QtWidgets>
#include "StackWidget.h"

namespace YSS::Editor {
	class HoverTip;

	class TextEdit :public QWidget, public StackWidget {
		Q_OBJECT;
	private:
		QTextEdit* Line;
		QTextEdit* Text;
		QHBoxLayout* Layout;
		qint32 LineCount = 0;
		qint8 TabWidth = 4;
		qint32 LastCursorLine = 0;
		QTextCursor LastCursor;
		QFont Font;
		QFontMetricsF* FontMetrics;
		QSyntaxHighlighter* Highlighter;
		bool ReloadTab = true;
		HoverTip* Hover = nullptr;
		QString FilePath;
		bool TextChanged = false;
	public:
		TextEdit(QWidget* parent = nullptr);
		void openFile(const QString& path);
		void saveFile(const QString& path = "");
	private:
		void onBlockCountChanged(qint32 count);
		virtual bool eventFilter(QObject* obj, QEvent* event) override;
		virtual void closeEvent(QCloseEvent* event) override;
		void onTabClicked(QKeyEvent* event);
		void onEnterClicked(QKeyEvent* event);
		void onMouseMove(QMouseEvent* event);
		void onCursorPositionChanged();
		void onTextChanged();
		virtual bool onWidgetClose() override;
		virtual QWidget* getWidget() override;
	};
}
