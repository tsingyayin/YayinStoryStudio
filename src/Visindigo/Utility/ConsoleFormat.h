#ifndef Visindigo_Utility_ConsoleFormat_h
#define Visindigo_Utility_ConsoleFormat_h
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtGui/qcolor.h>
#include "VICompileMacro.h"

// Main
namespace Visindigo::Utility {
	class VisindigoAPI ConsoleFormat {
	public:
		enum class Color {
			None, Black, Gray, Red, LightRed,
			Green, LightGreen, Yellow, LightYellow,
			Blue, LightBlue, Purple, LightPurple,
			Cyan, LightCyan, LightGray, White
		};
		enum class Style {
			Normal, Bold, Italic, Underline, Splash, Inverse, Strikethrough
		};
	public:
		static QString resetFormatStr();
	public:
		ConsoleFormat();
		ConsoleFormat(const QString& text, Color textColor = Color::None, QList<Style> styleList = { Style::Normal }, Color bgColor = Color::None);
		ConsoleFormat(const QString& text, Color textColor, Style style, Color bgColor = Color::None);
		ConsoleFormat(const QString& text, QColor textRgb, QList<Style> styleList = { Style::Normal }, QColor bgRgb = QColor());
		ConsoleFormat(const QString& text, QColor textRgb, Style style, QColor bgRgb = QColor());
	public:
		void setText(const QString& text);
		QString getText() const;
		void setTextColor(Color color);
		void setTextColor(QColor rgb);
		QColor getTextColor() const;
		void setTextStyle(QList<Style> styleList);
		void setTextStyle(Style style);
		QList<Style> getTextStyle() const;
		void setBackgroundColor(Color color);
		void setBackgroundColor(QColor rgb);
		QColor getBackgroundColor() const;
		QString toString() const;
		bool isTextColorEnabled() const;
		bool isBackgroundColorEnabled() const;
		bool isStyleEnabled() const;
	private:
		static QColor colorToRgb(Color color);
	private:
		QString Text;
		QColor TextColor;
		QColor BackgroundColor;
		QList<Style> StyleList = { Style::Normal };
		bool TextColorEnabled = false;
		bool BackgroundColorEnabled = false;
	};
}
#endif // Visindigo_Utility_ConsoleFormat_h
