#pragma once
#include "../Macro.h"
#include "./FileEditWidget.h"

// Forward declarations
class QTextEdit;
class QHBoxLayout;
class QSyntaxHighlighter;
namespace YSSCore::__Private__ {
	class TextEditPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI TextEdit :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
		friend class YSSCore::__Private__::TextEditPrivate;
	public:
		TextEdit(QWidget* parent = nullptr);
		virtual ~TextEdit();
	private:
		virtual bool eventFilter(QObject* obj, QEvent* event) override;
		virtual void closeEvent(QCloseEvent* event) override;
	protected:
		virtual bool onOpen(const QString& path) override;
		virtual bool onClose() override;
		virtual bool onSave(const QString& path = "") override;
		virtual bool onReload() override;
	private:
		YSSCore::__Private__::TextEditPrivate* d;
	};
}
