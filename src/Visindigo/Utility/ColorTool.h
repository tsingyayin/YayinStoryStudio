#ifndef Visindigo_Utility_ColorTool_h
#define Visindigo_Utility_ColorTool_h
#include "../Macro.h"
#include <QtGui/qcolor.h>
class QString;
namespace Visindigo::Utility {
	class VisindigoAPI ColorTool {
	public:
		enum ColorStrFormat {
			rgbCSS, // rgb(r, g, b)
			rgbaCSS, // rgba(r, g, b, a)
			rgbHex, // #RRGGBB
			rgbaHex, // #RRGGBBAA
			rgbHex12b, // #RRRGGGBBB
			rgbaHex16b, // #RRRRGGGGBBBB
			rgbInt, // integer 0 - 16777215
			rgbaInt // integer 0 - 4294967295
		};
	public:
		// warning: this function is only for 8-bit color channel, if you pass value out of range, the result is undefined.
		static constexpr bool isLightColor(const quint8& r, const quint8& g, const quint8& b) noexcept {
			return (((5 *g) + (2 * r) + b) > 1024);
		};
		static bool isLightColor(const QColor& color) noexcept;
		static QString toColorString(const QColor& color, ColorStrFormat format = rgbHex) noexcept;
	};
}
#endif