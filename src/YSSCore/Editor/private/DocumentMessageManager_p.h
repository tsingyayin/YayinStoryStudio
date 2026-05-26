#ifndef YSSCore_Editor_DocumentMessageManager_p_H
#define YSSCore_Editor_DocumentMessageManager_p_H
#include "Editor/SyntaxHighlighter.h"
#include <tuple>
namespace YSSCore::Editor {
	class DocumentMessage;
	class DocumentMessageManager;

	class DocumentMessagePrivate {
		friend class DocumentMessage;
		friend class DocumentMessageManager;
		friend class YSSCore::__Private__::DocumentMessageManagerPrivate;
	protected:
		DocumentMessage::MessageType Type;
		QString Message;
		qint32 LineNumber;
		qint32 ColumnNumber;
		qint32 Length;
		QString Code;
		QString fixAdvice;
		QUrl HelpUrl;
	};
}

namespace YSSCore::__Private__ {
	class DocumentMessageManagerPrivate {
	private:
		QMap<QString, QMap<qint32, QList<YSSCore::Editor::DocumentMessage>>> Messages;
		QMap<QString, std::tuple<qint32, qint32, qint32>> MessageCounts;
	public:
		static YSSCore::Editor::DocumentMessageManager* Instance;
		void clearMessagesForFile(const QString& filePath);
		void clearMessagesForBlock(const QString& filePath, qint32 blockNumber);
		void addMessage(const QString& filePath, qint32 blockNumber, const YSSCore::Editor::DocumentMessage& message);
		void moveMessageForward(const QString& filePath, qint32 deletedBlockStartNumber, qint32 count);
		void moveMessageBackward(const QString& filePath, qint32 insertedBlockStartNumber, qint32 count);
		void flushMessages(const QString& filePath, qint32 blockNumber);
		void flushMessages(const QString& filePath);
		bool hasMessage(const QString& filePath);
		bool hasMessage(const QString& filePath, qint32 lineNumber);
		QMap<qint32, QList<YSSCore::Editor::DocumentMessage>> getAllMessages(const QString& filePath);
		QList<YSSCore::Editor::DocumentMessage> getMessages(const QString& filePath, qint32 lineNumber);
		std::tuple<qint32, qint32, qint32> getMessageCount(const QString& filePath);
	};
}
#endif // YSSCore_Editor_DocumentMessageManager_p_H