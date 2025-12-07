#include <QtGui/qtextdocument.h>
#include "../private/TabCompleterProvider_p.h"
#include "../TabCompleterProvider.h"

namespace YSSCore::Editor {
	class TabCompleterItemPrivate {
		friend class TabCompleterItem;
	protected:
		TabCompleterItem::ItemType Type = TabCompleterItem::ItemType::Default;
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

	TabCompleterItem::TabCompleterItem(QString text, QString content, QString description, ItemType type, bool alignment)
		: d(new TabCompleterItemPrivate) {
		d->IconPath = "";
		d->Text = text;
		d->Description = description;
		d->Content = content;
		d->Type = type;
		d->Alignment = alignment;
		setType(type, true);
	}
	TabCompleterItem::~TabCompleterItem() {
		delete d;
	}

	VIMoveable_Impl(TabCompleterItem);

	VICopyable_Impl(TabCompleterItem);

	void TabCompleterItem::setIconPath(const QString& iconPath) {
		d->IconPath = iconPath;
	}

	void TabCompleterItem::setText(const QString& text) {
		d->Text = text;
	}

	void TabCompleterItem::setDescription(const QString& description) {
		d->Description = description;
	}

	void TabCompleterItem::setContent(const QString& content) {
		d->Content = content;
	}

	void TabCompleterItem::setType(ItemType type, bool redirectIcon) {
		if (redirectIcon) {
			switch (type) {
			case ItemType::Default:
				d->IconPath = ":/Visindigo/compiled/icon/default.png";
				break;
			case ItemType::Value:
				d->IconPath = ":/Visindigo/compiled/icon/value.png";
				break;
			case ItemType::Const:
				d->IconPath = ":/Visindigo/compiled/icon/const.png";
				break;
			case ItemType::Enum:
				d->IconPath = ":/Visindigo/compiled/icon/enum.png";
				break;
			case ItemType::Function:
				d->IconPath = ":/Visindigo/compiled/icon/function.png";
				break;
			case ItemType::Object:
				d->IconPath = ":/Visindigo/compiled/icon/object.png";
				break;
			case ItemType::Operator:
				d->IconPath = ":/Visindigo/compiled/icon/operator.png";
				break;
			default:
				d->IconPath = ":/Visindigo/compiled/icon/default.png";
			}
		}
		d->Type = type;
	}
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

	TabCompleterItem::ItemType TabCompleterItem::getType() const {
		return d->Type;
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