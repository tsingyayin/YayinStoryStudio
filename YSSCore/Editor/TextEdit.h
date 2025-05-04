#pragma once
#include <QtWidgets>
#include "FileEditWidget.h"

namespace YSSCore::Editor {
	class HoverTip;

	class TextEdit :public YSSCore::Editor::FileEditWidget {
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
		virtual ~TextEdit();
		virtual bool openFile(const QString& path) override;
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
		virtual QString getFilePath() const override;
		virtual QString getFileName() const override;
		virtual bool onClose() override;
		virtual bool onSave(const QString& path = "") override;
		virtual bool onSaveAs(const QString& path = "") override;
		virtual bool onReload() override;
		virtual QWidget* getWidget() override;
	};
}
