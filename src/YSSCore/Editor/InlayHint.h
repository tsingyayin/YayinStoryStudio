#ifndef YSSCore_Editor_InlayHint_h
#define YSSCore_Editor_InlayHint_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qstring.h>
#include <QtGui/qcolor.h>
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI InlayHint {
	private:
		qint32 PositionInBlock = 0;
		QString Text;
		QColor Color;
		qint32 Paddings = 8;
	public:
		InlayHint();
		InlayHint(qint32 positionInBlock, const QString& text, const QColor& color = QColor(), qint32 paddings = 8);
		~InlayHint();
	public:
		void setPositionInBlock(qint32 positionInBlock);
		void setText(const QString& text);
		void setColor(const QColor& color);
		void setPaddings(qint32 paddings);
		QString getText() const;
		QColor getColor() const;
		qint32 getPositionInBlock() const;
		qint32 getPaddings() const;
	};
}
#endif // YSSCore_Editor_InlayHint_h
