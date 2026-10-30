#include <QtCore/qstring.h>
#include "Utility/ColorTool.h"
namespace Visindigo::Utility {
	namespace {
		const QColor DangerLightColor(0xC0, 0x27, 0x1C);
		const QColor DangerDarkColor(0xFF, 0x6F, 0x61);
		const QColor SuccessLightColor(0x12, 0x8A, 0x21);
		const QColor SuccessDarkColor(0x3D, 0xDC, 0x63);
		constexpr int WarnBackgroundAlpha = 32;
		constexpr int WarnBorderAlpha = 140;
		constexpr int SelectionAlpha = 70;
		constexpr qreal DimMixRatio = 0.45;

		QColor blend(const QColor& from, const QColor& to, qreal ratio) {
			return QColor::fromRgbF(
				from.redF() * (1.0 - ratio) + to.redF() * ratio,
				from.greenF() * (1.0 - ratio) + to.greenF() * ratio,
				from.blueF() * (1.0 - ratio) + to.blueF() * ratio);
		}

		QColor withAlpha(QColor color, int alpha) {
			color.setAlpha(alpha);
			return color;
		}

		QColor cardOf(const QPalette& palette, bool light) {
			return light ? palette.color(QPalette::Base) : palette.color(QPalette::Window).lighter(115);
		}

		QColor highlightOf(const QPalette& palette) {
			QColor highlight = palette.color(QPalette::Highlight);
			if (!highlight.isValid()) {
				highlight = palette.color(QPalette::Accent);
			}
			return highlight;
		}

		QColor accentFillOf(const QPalette& palette, bool light) {
			const QColor highlight = highlightOf(palette);
			return light ? highlight : highlight.darker(118);
		}
	}

	/*!
		\class Visindigo::Utility::ColorTool
		\brief 这类提供一些和颜色相关的工具函数.
		\since Visindigo 0.13.0
		\inmodule Visindigo

		ColorTool集中了与颜色相关的纯函数工具。除了把颜色序列化成字符串以外，它还提供了一套
		从QPalette推导界面用色的规则，见 \l{PaletteColor}。
	*/

	/*!
		\enum Visindigo::Utility::ColorTool::ColorStrFormat
		\since Visindigo 0.13.0
		\value rgbCSS rgb(r, g, b)
		\value rgbaCSS rgba(r, g, b, a)
		\value rgbHex #RRGGBB
		\value rgbaHex #RRGGBBAA
		\value rgbHex12b #RRRGGGBBB
		\value rgbaHex16b #RRRRGGGGBBBB
		\value rgbInt 整数 0 - 16777215
		\value rgbaInt 整数 0 - 4294967295
	*/

	/*!
		\enum Visindigo::Utility::ColorTool::PaletteColor
		\since Visindigo 0.18.0

		可从一个QPalette推导出的界面用色。枚举名的字面量同时用于ThemeManager样式模板里的调色板令牌，
		即 \c{$[Name]} 形式。

		前 21 项与QPalette::ColorRole一一对应，枚举名也保持一致，直接读取调色板中的对应项。
		其后的项是界面层需要的派生色，由调色板中的若干颜色算出。

		\value WindowText 与\c{QPalette::WindowText}一致。
		\value Button 与\c{QPalette::Button}一致。
		\value Light 与\c{QPalette::Light}一致。
		\value Midlight 与\c{QPalette::Midlight}一致。
		\value Dark 与\c{QPalette::Dark}一致。
		\value Mid 与\c{QPalette::Mid}一致。
		\value Text 与\c{QPalette::Text}一致。
		\value BrightText 与\c{QPalette::BrightText}一致。
		\value ButtonText 与\c{QPalette::ButtonText}一致。
		\value Base 与\c{QPalette::Base}一致。
		\value Window 与\c{QPalette::Window}一致。
		\value Shadow 与\c{QPalette::Shadow}一致。
		\value Highlight 与\c{QPalette::Highlight}一致，取不到时回退到\c{QPalette::Accent}。
		\value HighlightedText 与\c{QPalette::HighlightedText}一致。
		\value Link 与\c{QPalette::Link}一致。
		\value LinkVisited 与\c{QPalette::LinkVisited}一致。
		\value AlternateBase 与\c{QPalette::AlternateBase}一致。
		\value ToolTipBase 与\c{QPalette::ToolTipBase}一致。
		\value ToolTipText 与\c{QPalette::ToolTipText}一致。
		\value PlaceholderText 与\c{QPalette::PlaceholderText}一致。
		\value Accent 与\c{QPalette::Accent}一致。
		\value Card 卡片类容器的底色。明亮主题取\c{Base}，深色主题取\c{Window}提亮15%。
		\value Inset 内凹区域（输入框、列表）的底色。明亮主题取\c{Window}压暗4%，深色主题取\c{Base}。
		\value Border 描边色。明亮主题取\c{Window}压暗20%，深色主题取\c{Window}提亮40%。
		\value Dim 次级文字色。由\c{WindowText}向\c{Card}混合45%得到。

		之所以不直接用\c{PlaceholderText}，是因为调色板里的这个颜色往往带透明度，
		序列化时会丢掉alpha，等于拿到了正文色。
		\value AccentFill 强调色的填充版本。深色主题下\c{Highlight}偏亮，铺浅色文字对比度不够，
		因此压暗18%；明亮主题下直接用\c{Highlight}。
		\value AccentHover 悬停态填充色，为\c{AccentFill}提亮12%。
		\value AccentPressed 按下态填充色，为\c{AccentFill}压暗12%。
		\value Danger 危险操作色。明亮主题为\c{#C0271C}，深色主题为\c{#FF6F61}。
		\value Success 成功状态色。明亮主题为\c{#128A21}，深色主题为\c{#3DDC63}。
		\value WarnBackground 警示区背景色，为\c{Danger}附加32的alpha。
		\value WarnBorder 警示区描边色，为\c{Danger}附加140的alpha。
		\value Selection 半透明的选中色，为\c{Highlight}附加70的alpha，用于铺在内容之上。
	*/

	/*!
		\fn constexpr bool Visindigo::Utility::ColorTool::isLightColor(const quint8& r, const quint8& g, const quint8& b) noexcept
		\since Visindigo 0.13.0
		\a r \a g \a b 分别是颜色的红、绿、蓝分量，范围是0-255。
		这个函数根据颜色的RGB值来判断它是浅色还是深色。

		这是一个由微软推荐的全整数快速表达式，它的唯一缺陷是只适用于8位颜色通道（0-255），如果你传入的值超出这个范围，结果是未定义的。
	*/

	/*!
		\since Visindigo 0.13.0
		\a color 要判断的颜色。

		return 判断颜色是浅色还是深色。

		对rgb分量版本函数的重载。
	*/
	bool ColorTool::isLightColor(const QColor& color) noexcept {
		return isLightColor(static_cast<quint8>(color.red()),
			static_cast<quint8>(color.green()),
			static_cast<quint8>(color.blue()));
	};

	/*!
		\since Visindigo 0.18.0
		\a palette 要判断的调色板。

		return 该调色板是否是浅色系。

		判定依据是 \c{QPalette::Window}，与 \l{paletteColor()} 内部使用的判据一致。
	*/
	bool ColorTool::isLightPalette(const QPalette& palette) noexcept {
		return isLightColor(palette.color(QPalette::Window));
	}

	/*!
		\since Visindigo 0.18.0
		\a palette 用于取色的调色板。
		\a color 需要的颜色项。

		return 从 \a palette 推导出的颜色。

		与QPalette::ColorRole对应的项直接读取对应颜色；派生项的算法见 \l{PaletteColor} 的逐项说明。
		传入无效调色板时返回无效颜色。
	*/
	QColor ColorTool::paletteColor(const QPalette& palette, PaletteColor color) noexcept {
		const bool light = isLightPalette(palette);
		switch (color) {
		case PaletteColor::WindowText: return palette.color(QPalette::WindowText);
		case PaletteColor::Button: return palette.color(QPalette::Button);
		case PaletteColor::Light: return palette.color(QPalette::Light);
		case PaletteColor::Midlight: return palette.color(QPalette::Midlight);
		case PaletteColor::Dark: return palette.color(QPalette::Dark);
		case PaletteColor::Mid: return palette.color(QPalette::Mid);
		case PaletteColor::Text: return palette.color(QPalette::Text);
		case PaletteColor::BrightText: return palette.color(QPalette::BrightText);
		case PaletteColor::ButtonText: return palette.color(QPalette::ButtonText);
		case PaletteColor::Base: return palette.color(QPalette::Base);
		case PaletteColor::Window: return palette.color(QPalette::Window);
		case PaletteColor::Shadow: return palette.color(QPalette::Shadow);
		case PaletteColor::Highlight: return highlightOf(palette);
		case PaletteColor::HighlightedText: return palette.color(QPalette::HighlightedText);
		case PaletteColor::Link: return palette.color(QPalette::Link);
		case PaletteColor::LinkVisited: return palette.color(QPalette::LinkVisited);
		case PaletteColor::AlternateBase: return palette.color(QPalette::AlternateBase);
		case PaletteColor::ToolTipBase: return palette.color(QPalette::ToolTipBase);
		case PaletteColor::ToolTipText: return palette.color(QPalette::ToolTipText);
		case PaletteColor::PlaceholderText: return palette.color(QPalette::PlaceholderText);
		case PaletteColor::Accent: return palette.color(QPalette::Accent);
		case PaletteColor::Card: return cardOf(palette, light);
		case PaletteColor::Inset:
			return light ? palette.color(QPalette::Window).darker(104) : palette.color(QPalette::Base);
		case PaletteColor::Border:
			return light ? palette.color(QPalette::Window).darker(120) : palette.color(QPalette::Window).lighter(140);
		case PaletteColor::Dim:
			return blend(palette.color(QPalette::WindowText), cardOf(palette, light), DimMixRatio);
		case PaletteColor::AccentFill: return accentFillOf(palette, light);
		case PaletteColor::AccentHover: return accentFillOf(palette, light).lighter(112);
		case PaletteColor::AccentPressed: return accentFillOf(palette, light).darker(112);
		case PaletteColor::Danger:
			return light ? DangerLightColor : DangerDarkColor;
		case PaletteColor::Success:
			return light ? SuccessLightColor : SuccessDarkColor;
		case PaletteColor::WarnBackground:
			return withAlpha(light ? DangerLightColor : DangerDarkColor, WarnBackgroundAlpha);
		case PaletteColor::WarnBorder:
			return withAlpha(light ? DangerLightColor : DangerDarkColor, WarnBorderAlpha);
		case PaletteColor::Selection:
			return withAlpha(highlightOf(palette), SelectionAlpha);
		}
		return QColor();
	}

	/*!
		\since Visindigo 0.13.0
		\a color 要转换的颜色
		\a format 字符串的格式，默认为rgbHex（#RRGGBB）。

		return 将颜色转换为字符串。

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
