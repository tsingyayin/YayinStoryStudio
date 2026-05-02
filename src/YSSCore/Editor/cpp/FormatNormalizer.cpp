#include "Editor/FormatNormalizer.h"
#include <QtGui/qtextdocument.h>
#include "Editor/TextEdit.h"

namespace YSSCore::Editor {
	class FormatNormalizerPrivate {
		friend class FormatNormalizer;
	protected:
		TextEdit* TextEditPtr = nullptr;
	};
	FormatNormalizer::FormatNormalizer(TextEdit* parent) : QObject(parent->getDocument()) {
		d = new FormatNormalizerPrivate();
		d->TextEditPtr = parent;
	}
	FormatNormalizer::~FormatNormalizer() {
		delete d;
	}
	QTextDocument* FormatNormalizer::getDocument() const {
		return d->TextEditPtr->getDocument();
	}
}