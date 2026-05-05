#ifndef YSSCore_Editor_FormatNormalizer_H
#define YSSCore_Editor_FormatNormalizer_H
#include <QtCore/qobject.h>
#include "YSSCoreCompileMacro.h"

class QTextDocument;
namespace YSSCore::Editor {
	class TextEdit;
	class FormatNormalizerPrivate;
	class YSSCoreAPI FormatNormalizer :public QObject{
	public:
		FormatNormalizer(TextEdit* parent);
		~FormatNormalizer();
		QTextDocument* getDocument() const;
		virtual QString onNormalize(qint32 lineNumber, const QString& content) = 0;
	private:
		FormatNormalizerPrivate* d;
	};
}
#endif // YSSCore_Editor_FormatNormalizer_H