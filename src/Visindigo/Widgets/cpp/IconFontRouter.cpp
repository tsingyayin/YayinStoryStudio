#include "Widgets/IconFontRouter.h"
#include "Widgets/private/IconFontRouter_SegoeFluent_p.h"
namespace Visindigo::__Private__ {
	IconFontRouter_SegoeFluent* IconFontRouter_SegoeFluent::Instance = nullptr;

	IconFontRouter_SegoeFluent* IconFontRouter_SegoeFluent::getRouter() {
		if (not Instance) {
			Instance = new IconFontRouter_SegoeFluent();
		}
		return Instance;
	}

	IconFontRouter_SegoeFluent::IconFontRouter_SegoeFluent() :IconFontRouter("Segoe Fluent Icons") {}

	IconFontRouter_SegoeFluent::~IconFontRouter_SegoeFluent() {
		Instance = nullptr;
	}

	QChar IconFontRouter_SegoeFluent::getChar(IconName iconName) {
		return QChar(static_cast<quint32>(iconName));
	}

}
namespace Visindigo::Widgets {
	class IconFontRouterPrivate {
		friend class IconFontRouter;
	protected:
		QString fontFamily;
	};
	IconFontRouter* IconFontRouter::getSegoeFluentRouter() {
		return Visindigo::__Private__::IconFontRouter_SegoeFluent::getRouter();
	}

	IconFontRouter::IconFontRouter(const QString& fontFamily) :d(new IconFontRouterPrivate()) {
		d->fontFamily = fontFamily;
	}

	QChar IconFontRouter::getChar(const QString& iconName) {
		auto enumValue = QMetaEnum::fromType<IconName>().keyToValue(iconName.toStdString().c_str());
		if (enumValue != -1) {
			return getChar(static_cast<IconName>(enumValue));
		}
		return QChar();
	}

	QString IconFontRouter::getChars(const QString& iconNames, const QString& splitChar) {
		if (iconNames.contains(splitChar)) {
			QStringList parts = iconNames.split(splitChar);
			QString output;
			for (auto p : parts) {
				output.append(getChar(p));
			}
			return output;
		}
		else {
			return getChar(iconNames);
		}
	}

	QString IconFontRouter::getChars(const QList<IconName>& iconNames, const QString& splitChar) {
		QString output;
		for (auto name : iconNames) {
			output.append(getChar(name));
		}
		return output;
	}
}

