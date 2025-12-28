#pragma once
#include <QString>
#include <QList>
#include <QByteArray>
#include "../Macro.h"
// Forward declarations
class QString;
class QColor;
// Main
namespace Visindigo::Utility {
	class VisindigoAPI Console {
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
		static QString getColorString(QString rawText, Console::Color color, QList<Console::Style> styleList = { Console::Style::Normal });
		static QString getColorString(QString rawText, Console::Color color, Console::Style style = Console::Style::Normal);
		static QString getColorString(QString rawText, QColor rgb, QList<Console::Style> styleList = { Console::Style::Normal });
		static QString getColorString(QString rawText, QColor rgb, Console::Style style = Console::Style::Normal);
		static QString getLine();
		static QString inWarningStyle(QString rawText);
		static QString inErrorStyle(QString rawText);
		static QString inSuccessStyle(QString rawText);
		static QString inNoticeStyle(QString rawText);
		static QString getRawText(QString colorString);
		static QString cmdColorToHtmlString(QString cmdColorString);
		static void print(QString msg);
		static void binary(const QByteArray& data);
		static QString binaryToString(const QByteArray& data);
		template <typename T> static inline void memory(const T* data) {
			binary(QByteArray((char*)data, sizeof(T)));
		};
		template <typename T> static inline QString memoryToString(const T* data) {
			return binaryToString(QByteArray((char*)data, sizeof(T)));
		};
		static unsigned int exec(const QString& cmd);
	};
}