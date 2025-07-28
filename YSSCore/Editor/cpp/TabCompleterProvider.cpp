#include "../TabCompleterProvider.h"
#include "../private/TabCompleterProvider_p.h"
#include <QtGui/qtextdocument.h>
#include "../TextEdit.h"
namespace YSSCore::Editor {
	class TabCompleterItemPrivate {
		friend class TabCompleterItem;
	protected:
		QString IconPath;
		QString Text;
		QString Description;
		QString Content;
		bool Alignment;
	};

	TabCompleterItem::TabCompleterItem()
		: d(new TabCompleterItemPrivate) {
		d->IconPath = "";
		d->Text = "";
		d->Description = "";
		d->Content = "";
		d->Alignment = true;
	}

	TabCompleterItem::TabCompleterItem(QString iconPath, QString text, QString description, QString content, bool alignment)
		: d(new TabCompleterItemPrivate) {
		d->IconPath = iconPath;
		d->Text = text;
		d->Description = description;
		d->Content = content;
		d->Alignment = alignment;
	}

	TabCompleterItem::~TabCompleterItem() {
		delete d;
	}

	VIMoveable_Impl(TabCompleterItem);

	VICopyable_Impl(TabCompleterItem);

	QString TabCompleterItem::getIconPath() const {
		return d->IconPath;
	}

	QString TabCompleterItem::getText() const {
		return d->Text;
	}

	QString TabCompleterItem::getDescription() const {
		return d->Description;
	}

	QString TabCompleterItem::getContent() const {
		return d->Content;
	}

	bool TabCompleterItem::isAlignment() const {
		return d->Alignment;
	}

	TabCompleterProvider::TabCompleterProvider(QTextDocument* doc)
		: QObject(doc), d(new YSSCore::__Private__::TabCompleterProviderPrivate) {
		d->Q = this;
		d->Document = doc;
	}

	TabCompleterProvider::~TabCompleterProvider() {
		delete d;
	}

	QTextDocument* TabCompleterProvider::getDocument() const {
		return d->Document;
	}
}