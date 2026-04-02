#ifndef YSSCore_Editor_DocumentMessageManager_h
#define YSSCore_Editor_DocumentMessageManager_h
#include "../Macro.h"
#include "Editor/SyntaxHighlighter.h"
namespace YSSCore::__Private__ {
	class DocumentMessageManagerPrivate;
}
namespace YSSCore::Editor {
	class YSSCoreAPI DocumentMessageManager :public QObject {
		friend class SyntaxHighlighter;
		friend class SyntaxHighlighterPrivate;
		Q_OBJECT;
	signals:
		void messageChanged(const QString& filePath);
		void messageChangedForLine(const QString& filePath, qint32 lineNumber);
	private:
		DocumentMessageManager();
	public:
		static DocumentMessageManager* getInstance();
		~DocumentMessageManager();
		bool hasMessage(const QString& filePath);
		bool hasMessage(const QString& filePath, qint32 lineNumber);
		QMap<qint32, QList<DocumentMessage>> getAllMessages(const QString& filePath);
		QList<DocumentMessage> getMessages(const QString& filePath, qint32 lineNumber);
	protected:
		YSSCore::__Private__::DocumentMessageManagerPrivate* d;
	};
}
#endif //DocumentMessageManager_h