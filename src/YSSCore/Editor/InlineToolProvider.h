#ifndef YSSCore_Editor_InlineToolProvider_h
#define YSSCore_Editor_InlineToolProvider_h
#include "YSSCoreCompileMacro.h"
#include <QtGui/qtextdocument.h>
namespace YSSCore::Editor {
	class YSSCoreAPI InlineToolProvider {
	public:
		InlineToolProvider(const QString& id);
		~InlineToolProvider();
	public:
		virtual QImage onInlineObjectRequested(const QTextBlock& block, qint32 positionInBlock, qint32 requestNotice = 0) = 0;
		QWidget* onInlineWidgetRequested(const QTextBlock& block, qint32 positionInBlock);
	};
}
#endif // YSSCore_Editor_InlineToolProvider_h
