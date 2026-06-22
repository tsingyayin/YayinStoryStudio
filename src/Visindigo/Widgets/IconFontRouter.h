#ifndef Visindigo_Widgets_IconFontRouter_h
#define Visindigo_Widgets_IconFontRouter_h
#include "VICompileMacro.h"
#include <QtCore/qmetaobject.h>
#include <QtCore/qstring.h>
#include <QtGui/qicon.h>
namespace Visindigo::Widgets {
	class IconFontRouterPrivate;
	class VisindigoAPI IconFontRouter {
		Q_GADGET;
	public:
#include "Widgets/private/IconNameEnum.hpp"
		Q_ENUM(IconName);
	public:
		static IconFontRouter* getSegoeFluentRouter();
	public:
		IconFontRouter(const QString& fontFamily);
		virtual ~IconFontRouter() = default;
		QChar getChar(const QString& iconName);
		QString getChars(const QString& iconNames, const QString& splitChar = ";");
		QString getChars(const QList<IconName>& iconNames, const QString& splitChar = ";");
	public:
		virtual QChar getChar(IconName iconName) = 0;
    private:
		IconFontRouterPrivate* d;
	};
}
#endif // Visindigo_Widgets_IconFontRouter_h