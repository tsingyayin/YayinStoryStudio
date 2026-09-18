#ifndef YSSCore_Editor_private_DocumentMessage_p_h
#define YSSCore_Editor_private_DocumentMessage_p_h
#include "Editor/DocumentMessage.h"
#include "Editor/CustomDocumentData.h"
namespace YSSCore::Editor {
	class DocumentMessagePrivate {
		friend class DocumentMessage;
		friend class TextEdit;
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
	class DocumentMessageList :public YSSCore::Editor::ICustomDocumentData {
	public:
		QList<YSSCore::Editor::DocumentMessage> Messages;
	};
}
#endif // YSSCore_Editor_private_DocumentMessage_p_h
