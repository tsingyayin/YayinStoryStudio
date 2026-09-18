#ifndef YSSCore_Editor_DocumentMessage_h
#define YSSCore_Editor_DocumentMessage_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
namespace YSSCore::Editor {
	class TextEdit;
	class DocumentMessagePrivate;

	class YSSCoreAPI DocumentMessage {
		friend class TextEdit;
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