#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtGui/qcolor.h>
#include "Utility/ConsoleFormat.h"
namespace Visindigo::Utility {
	/*!
		\class Visindigo::Utility::ConsoleFormat
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief ConsoleFormat是一个以非PIMPL值类形式表示控制台文本格式的类。

		ConsoleFormat用一段文本及其前景色、背景色与样式表示一段控制台内容。
		颜色统一以真彩QColor保存；通过Color枚举设置时，会映射为近似的真彩。
		toString()输出带ANSI控制字符的文本（\033[38;2;r;g;b前景、
		\033[48;2;r;g;b背景、样式码，结尾 \033[0m 复位），输出格式与
		Console::getColorString(rgb, styleList) 一致，并可由
		Console::cmdColorToHtmlString() 正确解析为HTML。

		此类从0.17.0开始，取代Visindigo::Utility::Console类中有关控制台文本格式的功能。
	*/
	ConsoleFormat::ConsoleFormat() = default;

	ConsoleFormat::ConsoleFormat(const QString& text, Color textColor, QList<Style> styleList, Color bgColor)
		: Text(text), StyleList(styleList) {
		setTextColor(textColor);
		setBackgroundColor(bgColor);
	}

	ConsoleFormat::ConsoleFormat(const QString& text, Color textColor, Style style, Color bgColor)
		: ConsoleFormat(text, textColor, QList<Style>({ style }), bgColor) {}

	ConsoleFormat::ConsoleFormat(const QString& text, QColor textRgb, QList<Style> styleList, QColor bgRgb)
		: Text(text), StyleList(styleList) {
		setTextColor(textRgb);
		setBackgroundColor(bgRgb);
	}

	ConsoleFormat::ConsoleFormat(const QString& text, QColor textRgb, Style style, QColor bgRgb)
		: ConsoleFormat(text, textRgb, QList<Style>({ style }), bgRgb) {}

	void ConsoleFormat::setText(const QString& text) {
		Text = text;
	}

	QString ConsoleFormat::getText() const {
		return Text;
	}

	void ConsoleFormat::setTextColor(Color color) {
		const QColor rgb = colorToRgb(color);
		TextColorEnabled = rgb.isValid();
		if (TextColorEnabled)
			TextColor = rgb;
	}

	void ConsoleFormat::setTextColor(QColor rgb) {
		TextColorEnabled = rgb.isValid();
		if (TextColorEnabled)
			TextColor = rgb;
	}

	QColor ConsoleFormat::getTextColor() const {
		return TextColorEnabled ? TextColor : QColor();
	}

	void ConsoleFormat::setTextStyle(QList<Style> styleList) {
		StyleList = styleList;
	}

	void ConsoleFormat::setTextStyle(Style style) {
		StyleList = QList<Style>({ style });
	}

	QList<ConsoleFormat::Style> ConsoleFormat::getTextStyle() const {
		return StyleList;
	}

	void ConsoleFormat::setBackgroundColor(Color color) {
		const QColor rgb = colorToRgb(color);
		BackgroundColorEnabled = rgb.isValid();
		if (BackgroundColorEnabled)
			BackgroundColor = rgb;
	}

	void ConsoleFormat::setBackgroundColor(QColor rgb) {
		BackgroundColorEnabled = rgb.isValid();
		if (BackgroundColorEnabled)
			BackgroundColor = rgb;
	}

	QColor ConsoleFormat::getBackgroundColor() const {
		return BackgroundColorEnabled ? BackgroundColor : QColor();
	}

	/*!
		\since Visindigo 0.17.0

		返回固定的SGR复位控制码，用于结束一段带格式的文本。
	*/
	QString ConsoleFormat::resetFormatStr() {
		return QStringLiteral("\033[0m");
	}

	/*!
		\since Visindigo 0.17.0

		转为带ANSI控制字符的文本。无任何颜色与样式时原样返回文本，不附加控制字符；
		否则以前景38;2、背景48;2与各样式码组成SGR序列，并以 resetFormatStr() 复位结尾。
	*/
	QString ConsoleFormat::toString() const {
		if (not TextColorEnabled && not BackgroundColorEnabled && not isStyleEnabled()) {
			return Text;
		}
		QString codes;
		auto appendCode = [&codes](const QString& code) {
			if (not codes.isEmpty())
				codes += QChar(';');
			codes += code;
		};
		if (TextColorEnabled && TextColor.isValid())
			appendCode(QString::asprintf("38;2;%d;%d;%d", TextColor.red(), TextColor.green(), TextColor.blue()));
		if (BackgroundColorEnabled && BackgroundColor.isValid())
			appendCode(QString::asprintf("48;2;%d;%d;%d", BackgroundColor.red(), BackgroundColor.green(), BackgroundColor.blue()));
		for (Style style : StyleList) {
			switch (style) {
			case Style::Normal:
				break; // 不输出 0，避免覆盖上面的颜色设置
			case Style::Bold:
				appendCode(QStringLiteral("1"));
				break;
			case Style::Italic:
				appendCode(QStringLiteral("3"));
				break;
			case Style::Underline:
				appendCode(QStringLiteral("4"));
				break;
			case Style::Splash:
				appendCode(QStringLiteral("5"));
				break;
			case Style::Inverse:
				appendCode(QStringLiteral("7"));
				break;
			case Style::Strikethrough:
				appendCode(QStringLiteral("9"));
				break;
			}
		}
		if (codes.isEmpty())
			return Text;
		return QStringLiteral("\033[") + codes + QStringLiteral("m") + Text + resetFormatStr();
	}

	bool ConsoleFormat::isTextColorEnabled() const {
		return TextColorEnabled;
	}

	bool ConsoleFormat::isBackgroundColorEnabled() const {
		return BackgroundColorEnabled;
	}

	bool  ConsoleFormat::isStyleEnabled() const {
		return not StyleList.isEmpty() && not (StyleList.size() == 1 && StyleList.first() == Style::Normal);
	}
	QColor ConsoleFormat::colorToRgb(Color color) {
		switch (color) {
		case Color::Black: return QColor(0, 0, 0);
		case Color::Gray: return QColor(128, 128, 128);
		case Color::Red: return QColor(128, 0, 0);
		case Color::LightRed: return QColor(255, 0, 0);
		case Color::Green: return QColor(0, 128, 0);
		case Color::LightGreen: return QColor(0, 255, 0);
		case Color::Yellow: return QColor(128, 128, 0);
		case Color::LightYellow: return QColor(255, 255, 0);
		case Color::Blue: return QColor(0, 0, 128);
		case Color::LightBlue: return QColor(0, 0, 255);
		case Color::Purple: return QColor(128, 0, 128);
		case Color::LightPurple: return QColor(255, 0, 255);
		case Color::Cyan: return QColor(0, 128, 128);
		case Color::LightCyan: return QColor(0, 255, 255);
		case Color::LightGray: return QColor(192, 192, 192);
		case Color::White: return QColor(255, 255, 255);
		case Color::None:
		default:
			return QColor(); // 无效色 = 无颜色
		}
	}
}
