#ifndef YSSCore_Editor_DocumentMessageManager_p_H
#define YSSCore_Editor_DocumentMessageManager_p_H
#include "Editor/SyntaxHighlighter.h"
namespace YSSCore::Editor {
	class DocumentMessageManager;
}
namespace YSSCore::__Private__ {
	class DocumentMessageManagerPrivate {
	private:
		QMap<QString, QMap<qint32, QList<YSSCore::Editor::DocumentMessage>>> Messages;
	public:
		static YSSCore::Editor::DocumentMessageManager* Instance;
		void clearMessagesForFile(const QString& filePath);
		void clearMessagesForBlock(const QString& filePath, qint32 blockNumber);
		void addMessage(const QString& filePath, qint32 blockNumber, const YSSCore::Editor::DocumentMessage& message);
		void flushMessages(const QString& filePath, qint32 blockNumber);
		void flushMessages(const QString& filePath);
		bool hasMessage(const QString& filePath);
		bool hasMessage(const QString& filePath, qint32 lineNumber);
		QMap<qint32, QList<YSSCore::Editor::DocumentMessage>> getAllMessages(const QString& filePath);
		QList<YSSCore::Editor::DocumentMessage> getMessages(const QString& filePath, qint32 lineNumber);
	};
}
#endif // YSSCore_Editor_DocumentMessageManager_p_H