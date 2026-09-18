#ifndef YSSCore_Editor_TextEdit_h
#define YSSCore_Editor_TextEdit_h
#include "YSSCoreCompileMacro.h"
#include <tuple>
#include <typeindex>
#include "Editor/FileEditWidget.h"
#include <QtCore/qmap.h>
#include <QtGui/qtextdocument.h>
#include "Editor/DocumentMessage.h"
#include "Editor/TabCompleterProvider.h"
#include "Editor/CustomDocumentData.h"
// Forward declarations
class QTextEdit;
class QHBoxLayout;
class QSyntaxHighlighter;

namespace YSSCore::__Private__ {
	class TextEditPrivate;
	class DocumentOverviewLabel;
}
namespace YSSCore::Editor {
	class TabCompleterProvider;
	class HoverInfoProvider;
	class SyntaxHighlighter;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI TextEdit :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
		friend class YSSCore::__Private__::TextEditPrivate;
		friend class YSSCore::__Private__::DocumentOverviewLabel;
		friend class HoverInfoProvider;
		friend class SyntaxHighlighter;
	signals:
		void fontSizeChanged(qint32 newSize);
		void textFontChanged(const QFont& newFont);
		void cursorPositionChanged(const QTextCursor& cursor);
		void modifySuggestionAccepted(qint32 lineNumber, const QString& suggestion);
		void modifySuggestionRejected(qint32 lineNumber, const QString& suggestion);
		void ghostTextAccepted(qint32 insertLine, const QStringList& ghostText);
		void ghostTextRejected(qint32 insertLine, const QStringList& ghostText);
		void messageChanged();
		void messageChangedForLine(qint32 lineNumber);
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
		QTextCursor getTextCursor() const;
		qint32 getFontSize() const;
		void setFontSize(qint32 size);
		void setTextFont(const QFont& font);
		QFont getTextFont() const;
		void setTabWidth(qint32 width);
		qint32 getTabWidth() const;
		QList<QTextCursor> findAll(const QString& source, bool sourceAsRe = false, QTextDocument::FindFlags options = QTextDocument::FindFlags(), bool multiSelection = false) const;
		void clearFindAllSelection();
		QTextCursor findNext(const QString& text, bool sourceAsRe = false, qint32 from = -1, QTextDocument::FindFlags options = QTextDocument::FindFlags(), bool relocate = false) const;
		qint32 replaceAll(const QString& text, const QString& newText, bool textAsRe = false, QTextDocument::FindFlags options = QTextDocument::FindFlags());
		bool replaceNext(const QString& text, const QString& newText, bool textAsRe = false, qint32 from = -1, QTextDocument::FindFlags options = QTextDocument::FindFlags(), bool relocate = false);
		void showFindAndReplace();
		void setCompleterLevel(TabCompleterItem::CompleterLevel level);
		TabCompleterItem::CompleterLevel getCompleterLevel() const;
		void setCompleterTypeFilter(TabCompleterItem::ItemTypes filter);
		TabCompleterItem::ItemTypes getCompleterTypeFilter() const;
		void setReadOnly(bool readOnly);
		bool isReadOnly() const;
		SyntaxHighlighter* getSyntaxHighlighter() const;
	public:
		ICustomDocumentData* getBlockData(qint32 blockNumber, const std::type_index& type) const;
		void setBlockData(qint32 blockNumber, const std::type_index& type, ICustomDocumentData* data);
		bool hasBlockData(qint32 blockNumber, const std::type_index& type) const;
		void removeBlockData(qint32 blockNumber, const std::type_index& type);
		void removeAllBlockData(const std::type_index& type);
		QMap<qint32, ICustomDocumentData*> getAllBlockData(const std::type_index& type) const;
	public:
		template<typename T> T* getBlockData(qint32 blockNumber) const {
			return static_cast<T*>(getBlockData(blockNumber, std::type_index(typeid(T))));
		}
		template<typename T> void setBlockData(qint32 blockNumber, T* data) {
			setBlockData(blockNumber, std::type_index(typeid(T)), data);
		}
		template<typename T> bool hasBlockData(qint32 blockNumber) const {
			return hasBlockData(blockNumber, std::type_index(typeid(T)));
		}
		template<typename T> void removeBlockData(qint32 blockNumber) {
			removeBlockData(blockNumber, std::type_index(typeid(T)));
		}
		template<typename T> void removeAllBlockData() {
			removeAllBlockData(std::type_index(typeid(T)));
		}
		template<typename T> QMap<qint32, T*> getAllBlockData() const {
			QMap<qint32, T*> result;
			const QMap<qint32, ICustomDocumentData*> raw = getAllBlockData(std::type_index(typeid(T)));
			for (auto it = raw.begin(); it != raw.end(); ++it) {
				result.insert(it.key(), static_cast<T*>(it.value()));
			}
			return result;
		}
	public:
		void clearMessages(qint32 lineNumber);
		void addMessage(qint32 lineNumber, const DocumentMessage& message);
		bool hasMessage() const;
		bool hasMessage(qint32 lineNumber) const;
		QList<DocumentMessage> getMessages(qint32 lineNumber) const;
		QMap<qint32, QList<DocumentMessage>> getAllMessages() const;
		std::tuple<qint32, qint32, qint32> getMessageCount();
	protected:
		virtual bool onCursorToPosition(qint32 lineNumber, qint32 column) override;
		virtual bool onOpen(const QString& path) override;
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