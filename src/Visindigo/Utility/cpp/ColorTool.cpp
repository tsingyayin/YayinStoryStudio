#include "Utility/ColorTool.h"
#include <QtCore/qstring.h>
namespace Visindigo::Utility {
	/*!
		\class Visindigo::Utility::ColorTool
		\brief 这类提供一些和颜色相关的工具函数。
		\since Visindigo 0.13.0
		\inmodule Visindigo
	*/

	/*!
		\fn constexpr bool Visindigo::Utility::ColorTool::isLightColor(const quint8& r, const quint8& g, const quint8& b) noexcept
		\since Visindigo 0.13.0
		\a r、\a g和\b分别是颜色的红、绿、蓝分量，范围是0-255。
		这个函数根据颜色的RGB值来判断它是浅色还是深色。

		这是一个由微软推荐的全整数快速表达式，它的唯一缺陷是只适用于8位颜色通道（0-255），如果你传入的值超出这个范围，结果是未定义的。
	*/

	/*!
		\since Visindigo 0.13.0
		对rgb分量版本函数的重载。
	*/
	bool ColorTool::isLightColor(const QColor& color) noexcept {
		return isLightColor(static_cast<quint8>(color.red()),
			static_cast<quint8>(color.green()),
			static_cast<quint8>(color.blue()));
	};

	/*!
		\since Visindigo 0.13.0
		将颜色转换为字符串的函数。
		\a color是要转换的颜色，\a format是字符串的格式，默认为rgbHex（#RRGGBB）。
		这个函数支持多种格式，包括CSS的rgb()和rgba()函数、十六进制表示法以及整数表示法。
	*/
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