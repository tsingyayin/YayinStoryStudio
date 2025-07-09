#pragma once
#include "../Macro.h"
#include "FileEditWidget.h"
#include <QtGui/qtextcursor.h>
class QTextEdit;
class QHBoxLayout;
class QSyntaxHighlighter;

namespace YSSCore::Editor {
	class HoverTip;
	class TextEditPrivate;
	class YSSCoreAPI TextEdit :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
		VImpl(TextEdit);
	public:
		TextEdit(QWidget* parent = nullptr);
		virtual ~TextEdit();
	private:
		void onBlockCountChanged(qint32 count);
		virtual bool eventFilter(QObject* obj, QEvent* event) override;
		virtual void closeEvent(QCloseEvent* event) override;
		void onTabClicked(QKeyEvent* event);
		void onEnterClicked(QKeyEvent* event);
		void onMouseMove(QMouseEvent* event);
		void onCursorPositionChanged();
	protected:
		virtual bool onOpen(const QString& path) override;
		virtual bool onClose() override;
		virtual bool onSave(const QString& path = "") override;
		virtual bool onReload() override;
	};
}
