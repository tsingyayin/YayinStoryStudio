#ifndef YSSCore_Editor_DocumentMessage_h
#define YSSCore_Editor_DocumentMessage_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
namespace YSSCore::__Private__ {
	class DocumentMessageManagerPrivate;
}
namespace YSSCore::Editor {
	class TextEdit;
	class DocumentMessageManager;
	class DocumentMessagePrivate;

	class YSSCoreAPI DocumentMessage {
		friend class DocumentMessageManager;
		friend class YSSCore::__Private__::DocumentMessageManagerPrivate;
	public:
		enum MessageType {
			Error = 0,
			Warning = 1,
			Info = 2
		};
	public:
		DocumentMessage(MessageType type, const QString& message, qint32 lineNumber = -1,
			qint32 columnNumber = -1, qint32 length = -1, const QString& code = "", const QUrl& helpUrl = QUrl(),
			const QString& fixAdvice = "");
		VIMoveable(DocumentMessage);
		VICopyable(DocumentMessage);
		~DocumentMessage();
	public:
		MessageType getType() const;
		QString getMessage() const;
		int getLineNumber() const;
		int getColumnNumber() const;
		int getLength() const;
		QString getCode() const;
		QUrl getHelpUrl() const;
		QString getFixAdvice() const;
		QString toString() const;
	protected:
		DocumentMessagePrivate* d;
	};
}
#endif // YSSCore_Editor_DocumentMessage_h