#ifndef Visindigo_Utility_ColorTool_h
#define Visindigo_Utility_ColorTool_h
#include "VICompileMacro.h"
#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>
#include <QtGui/qpalette.h>
class QString;
namespace Visindigo::Utility {
	class VisindigoAPI ColorTool {
		Q_GADGET;
	public:
		enum ColorStrFormat {
			rgbCSS,
			rgbaCSS,
			rgbHex,
			rgbaHex,
			rgbHex12b,
			rgbaHex16b,
			rgbInt,
			rgbaInt
		};
		enum class PaletteColor {
			WindowText, Button, Light, Midlight, Dark, Mid, Text, BrightText,
			ButtonText, Base, Window, Shadow, Highlight, HighlightedText,
			Link, LinkVisited, AlternateBase, ToolTipBase, ToolTipText,
			PlaceholderText, Accent,
			Card, Inset, Border, Dim, AccentFill, AccentHover, AccentPressed,
			Danger, Success, WarnBackground, WarnBorder, Selection
		};
		Q_ENUM(PaletteColor)
	public:
		static constexpr bool isLightColor(const quint8& r, const quint8& g, const quint8& b) noexcept {
			return (((5 * g) + (2 * r) + b) > 1024);
		};
		static bool isLightColor(const QColor& color) noexcept;
		static bool isLightPalette(const QPalette& palette) noexcept;
		static QColor paletteColor(const QPalette& palette, PaletteColor color) noexcept;
		static QString toColorString(const QColor& color, ColorStrFormat format = rgbHex) noexcept;
	};
}
#endif // Visindigo_Utility_ColorTool_h
