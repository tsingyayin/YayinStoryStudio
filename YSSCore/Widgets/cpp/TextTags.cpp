#include "../TextTags.h"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
namespace YSSCore::Widgets {
	class TextTagsPrivate {
		friend class TextTags;
	protected:
		QVBoxLayout* Layout;
		QList<QHBoxLayout*> LineLayouts;
		QList<QLabel*> Labels;
		QStringList Tags;
	};

	TextTags::TextTags(QWidget* parent) :QFrame(parent) {
		d = new TextTagsPrivate;
		d->Layout = new QVBoxLayout(this);
	}
	TextTags::~TextTags() {
		delete d;
	}
	QStringList TextTags::getAllTags() {
		return d->Tags;
	}
	void TextTags::addTag(const QString& tag) {
		d->Tags.append(tag);
	}
	qint32 TextTags::indexOf(const QString& tag) {
		return d->Tags.indexOf(tag);
	}

	void TextTags::removeAt(qint32 index) {
		d->Tags.removeAt(index);
	}

	void TextTags::resizeEvent(QResizeEvent* event) {
	}
}