#include "Utility/ColorTool.h"
#include <QtCore/qstring.h>
namespace Visindigo::Utility {
	bool ColorTool::isLightColor(const QColor& color) noexcept {
		return isLightColor(static_cast<quint8>(color.red()),
			static_cast<quint8>(color.green()),
			static_cast<quint8>(color.blue()));
	};

	QString ColorTool::toColorString(const QColor& color, ColorStrFormat format) noexcept {
		switch (format) {
		case rgbCSS:
			return QString("rgb(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
		case rgbaCSS:
			return QString("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
		case rgbHex:
			return QString("#%1%2%3")
				.arg(color.red(), 2, 16, QChar('0'))
				.arg(color.green(), 2, 16, QChar('0'))
				.arg(color.blue(), 2, 16, QChar('0')).toUpper();
		case rgbaHex:
			return QString("#%1%2%3%4")
				.arg(color.red(), 2, 16, QChar('0'))
				.arg(color.green(), 2, 16, QChar('0'))
				.arg(color.blue(), 2, 16, QChar('0'))
				.arg(color.alpha(), 2, 16, QChar('0')).toUpper();
		case rgbHex12b:
			return QString("#%1%2%3")
				.arg(color.red() * 17, 3, 16, QChar('0'))
				.arg(color.green() * 17, 3, 16, QChar('0'))
				.arg(color.blue() * 17, 3, 16, QChar('0')).toUpper();
		case rgbaHex16b:
			return QString("#%1%2%3%4")
				.arg(color.red() * 257, 4, 16, QChar('0'))
				.arg(color.green() * 257, 4, 16, QChar('0'))
				.arg(color.blue() * 257, 4, 16, QChar('0'))
				.arg(color.alpha() * 257, 4, 16, QChar('0')).toUpper();
		case rgbInt:
			return QString::number((color.red() << 16) | (color.green() << 8) | (color.blue()));
		case rgbaInt:
			return QString::number((color.red() << 24) | (color.green() << 16) | (color.blue() << 8) | (color.alpha()));
		default:
			return QString();
		}
	}
}