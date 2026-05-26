#ifndef YSSCore_Editor_DocumentMessageManager_h
#define YSSCore_Editor_DocumentMessageManager_h
#include "YSSCoreCompileMacro.h"
#include "Editor/DocumentMessage.h"
#include <QtCore/qobject.h>
#include <tuple>
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
		std::tuple<qint32, qint32, qint32> getMessageCount(const QString& filePath);
	protected:
		YSSCore::__Private__::DocumentMessageManagerPrivate* d;
	};
}
#endif //DocumentMessageManager_h