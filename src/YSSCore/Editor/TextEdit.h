#ifndef YSSCore_Editor_TextEdit_h
#define YSSCore_Editor_TextEdit_h
#include "YSSCoreCompileMacro.h"
#include "Editor/FileEditWidget.h"
#include <QtGui/qtextdocument.h>

// Forward declarations
class QTextEdit;
class QHBoxLayout;
class QSyntaxHighlighter;

namespace YSSCore::__Private__ {
	class TextEditPrivate;
}
namespace YSSCore::Editor {
	class TabCompleterProvider;
	class HoverInfoProvider;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI TextEdit :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
		friend class YSSCore::__Private__::TextEditPrivate;
		friend class HoverInfoProvider;
		friend class SyntaxHighlighter;
	public:
		TextEdit(QWidget* parent = nullptr);
		virtual ~TextEdit();
		void setHoverArea(QWidget* area);
		void setPlainText(const QString& text);
		QString getPlainText() const;
		void moveCursorToLine(int lineNumber);
		int getCurrentLineNumber() const;
		void setHoverTimeout(qint32 ms);
		qint32 getHoverTimeout() const;
		void setTabReload(bool reload);
		bool isTabReload() const;
		QTextDocument* getDocument() const;
		qint32 getFontSize() const;
		void setFontSize(qint32 size);
		QList<QTextCursor> findAll(const QString& source, bool sourceAsRe = false, QTextDocument::FindFlags options = QTextDocument::FindFlags(), bool multiSelection = false) const;
		void clearFindAllSelection();
		QTextCursor findNext(const QString& text, bool sourceAsRe = false, qint32 from = -1, QTextDocument::FindFlags options = QTextDocument::FindFlags(), bool relocate = false) const;
		qint32 replaceAll(const QString& text, const QString& newText, bool textAsRe = false, QTextDocument::FindFlags options = QTextDocument::FindFlags());
		bool replaceNext(const QString& text, const QString& newText, bool textAsRe = false, qint32 from = -1, QTextDocument::FindFlags options = QTextDocument::FindFlags(), bool relocate = false);
		void showFindAndReplace();
	protected:
		virtual bool onCursorToPosition(qint32 lineNumber, qint32 column) override;
		virtual bool onOpen(const QString& path) override;
		virtual bool onClose() override;
		virtual bool onSave(const QString& path = "") override;
		virtual bool onReload() override;
		virtual bool onCopy() override;
		virtual bool onCut() override;
		virtual bool onPaste() override;
		virtual bool onUndo() override;
		virtual bool onRedo() override;
		virtual bool onSelectAll() override;
	protected:
		YSSCore::__Private__::TextEditPrivate* d;
	};
}
#endif // YSSCore_Editor_TextEdit_h