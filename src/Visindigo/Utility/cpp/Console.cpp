#include "../Console.h"
#include <QtCore/qstring.h>
#include <QtGui/qcolor.h>
#include <iostream>
#include <QtCore/qregularexpression.h>
#include <QtCore/qsystemdetection.h>

namespace Visindigo::Utility {
	/*!
	\class Visindigo::Utility::Console
	\inmodule Visindigo
	\brief 此类为YayinStoryStudio提供初级的控制台快捷功能。
	\since Visindigo 0.10.0

	VIConsole提供输出到控制台时可能需要的颜色控制、格式控制等功能，并提供了一个用于快速打印二进制数据的函数QString::printBinary()。

*/
/*!
	\enum Visindigo::Utility::Console::Color
	\since Visindigo 0.10.0
	\value None 无颜色
	\value Black 黑色
	\value Gray 灰色
	\value Red 红色
	\value LightRed 亮红色
	\value Green 绿色
	\value LightGreen 亮绿色
	\value Yellow 黄色
	\value LightYellow 亮黄色
	\value Blue 蓝色
	\value LightBlue 亮蓝色
	\value Purple 紫色
	\value LightPurple 亮紫色
	\value Cyan 青色
	\value LightCyan 亮青色
	\value LightGray 亮灰色
	\value White 白色
*/
/*!
	\enum Visindigo::Utility::Console::Style
	\since Visindigo 0.10.0
	\value Normal 正常
	\value Bold 粗体
	\value Italic 斜体
	\value Underline 下划线
	\value Splash 闪烁
	\value Inverse 反色
	\value Strikethrough 删除线
*/
/*!
	\since Visindigo 0.10.0
	\a rawText 为要设置颜色的文本。
	\a color 为颜色。
	\a styleList 为样式列表。
*/
	QString Console::getColorString(QString rawText, Console::Color color, QList<Console::Style> styleList) {
		QString styleString = "";
		switch (color) {
		case Color::None:
			styleString = "\033[0";
			break;
		case Color::Black:
			styleString = "\033[30";
			break;
		case Color::Gray:
			styleString = "\033[1;30";
			break;
		case Color::Red:
			styleString = "\033[31";
			break;
		case Color::LightRed:
			styleString = "\033[1;31";
			break;
		case Color::Green:
			styleString = "\033[32";
			break;
		case Color::LightGreen:
			styleString = "\033[1;32";
			break;
		case Color::Yellow:
			styleString = "\033[33";
			break;
		case Color::LightYellow:
			styleString = "\033[1;33";
			break;
		case Color::Blue:
			styleString = "\033[34";
			break;
		case Color::LightBlue:
			styleString = "\033[1;34";
			break;
		case Color::Purple:
			styleString = "\033[35";
			break;
		case Color::LightPurple:
			styleString = "\033[1;35";
			break;
		case Color::Cyan:
			styleString = "\033[36";
			break;
		case Color::LightCyan:
			styleString = "\033[1;36";
			break;
		case Color::LightGray:
			styleString = "\033[37";
			break;
		case Color::White:
			styleString = "\033[1;37";
			break;
		default:
			styleString = "\033[0";
			break;
		}
		for (int i = 0; i < styleList.size(); i++) {
			switch (styleList[i]) {
			case Style::Normal:
				styleString += ";0";
				break;
			case Style::Bold:
				styleString += ";1";
				break;
			case Style::Italic:
				styleString += ";3";
				break;
			case Style::Underline:
				styleString += ";4";
				break;
			case Style::Splash:
				styleString += ";5";
				break;
			case Style::Inverse:
				styleString += ";7";
				break;
			case Style::Strikethrough:
				styleString += ";9";
				break;
			}
		}
		styleString += "m";
		return styleString + rawText + "\033[0m";
	}

	/*!
		\overload
		\since Visindigo 0.13.0

		允许只设置一个样式。
	*/
	QString Console::getColorString(QString rawText, Console::Color color, Console::Style style) {
		return getColorString(rawText, color, QList<Console::Style>({ style }));
	}

	/*!
		\overload
		\since Visindigo 0.13.0

		允许使用RGB颜色。
	*/
	QString Console::getColorString(QString rawText, QColor rgb, QList<Console::Style> styleList) {
		QString styleString = QString::asprintf("\033[38;2;%d;%d;%d", rgb.red(), rgb.green(), rgb.blue());
		for (int i = 0; i < styleList.size(); i++) {
			switch (styleList[i]) {
			case Style::Normal:
				styleString += ";0";
				break;
			case Style::Bold:
				styleString += ";1";
				break;
			case Style::Italic:
				styleString += ";3";
				break;
			case Style::Underline:
				styleString += ";4";
				break;
			case Style::Splash:
				styleString += ";5";
				break;
			case Style::Inverse:
				styleString += ";7";
				break;
			case Style::Strikethrough:
				styleString += ";9";
				break;
			}
		}
		styleString += "m";
		return styleString + rawText + "\033[0m";
	}

	/*!
		\overload
		\since Visindigo 0.13.0

		允许在只设置一个样式的情况下使用RGB颜色。
	*/
	QString Console::getColorString(QString rawText, QColor rgb, Console::Style style) {
		return getColorString(rawText, rgb, QList<Console::Style>({ style }));
	}

	/*!
		\since Visindigo 0.13.0

		获取一行输入。
	*/
	QString Console::getLine() {
		std::string commandStd;
		std::getline(std::cin, commandStd);
		return QString::fromLocal8Bit(QByteArray::fromStdString(commandStd));
	}

	/*!
		\since Visindigo 0.13.0
		\a rawText 为要设置样式的文本。

		返回一个带有警告样式的文本，即黄色加粗。
	*/
	QString Console::inWarningStyle(QString rawText) {
		return getColorString(rawText, QColor(255, 253, 85), Style::Bold);
	}

	/*!
		\since Visindigo 0.13.0
		\a rawText 为要设置样式的文本。

		返回一个带有错误样式的文本，即红色加粗。
	*/
	QString Console::inErrorStyle(QString rawText) {
		return getColorString(rawText, Color::Red, Style::Bold);
	}

	/*!
		\since Visindigo 0.13.0
		\a rawText 为要设置样式的文本。

		返回一个带有成功样式的文本，即绿色加粗。
	*/
	QString Console::inSuccessStyle(QString rawText) {
		return getColorString(rawText, Color::Green, Style::Bold);
	}

	/*!
		\since Visindigo 0.13.0
		\a rawText 为要设置样式的文本。

		返回一个带有通知样式的文本，即蓝色加粗。
	*/
	QString Console::inNoticeStyle(QString rawText) {
		return getColorString(rawText, Color::Blue, Style::Bold);
	}

	/*!
		\since Visindigo 0.13.0
		\a colorString 为带有颜色样式的文本。

		返回一个不带颜色样式的文本。
	*/
	QString Console::getRawText(QString colorString) {
		QString rawText = colorString;
		rawText = rawText.remove(QRegularExpression("\u001B\\[[(0-9)*;*]*m"));
		return rawText;
	}

	/*!
		\since Visindigo 0.13.0
		\a cmdColorString 为带有颜色样式的文本。
		返回一个HTML格式的字符串，适用于在HTML环境中显示。
	*/
	QString Console::cmdColorToHtmlString(QString cmdColorString) {
		QString htmlString = cmdColorString.toHtmlEscaped();
		htmlString = htmlString.replace("\r\n", "\n");
		htmlString = htmlString.replace("\r", "\n");
		htmlString = htmlString.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
		htmlString = htmlString.replace(" ", "&nbsp;");
		htmlString = htmlString.replace("\n", "<br>");
		if (!htmlString.contains("\033")) {
			return QString("<span>%1</span>").arg(htmlString);
		}
		htmlString = htmlString.replace("\033[0m", "</span>");
		QRegularExpression re("\033\\[(.*?)m");
		QRegularExpressionMatchIterator i = re.globalMatch(htmlString);
		while (i.hasNext()) {
			QRegularExpressionMatch match = i.next();
			QStringList params = match.captured(1).split(";");
			QString styleString = "<span style=\"";
			for (int i = 0; i < params.size(); i++) {
				int code = params[i].toInt();
				switch (code) {
				case 38:
					if (code == 38 && params.size() >= 5 && params[1] == "2" || i == 0) {
						int r = params[2].toInt();
						int g = params[3].toInt();
						int b = params[4].toInt();
						styleString += QString("color: rgb(%1, %2, %3);").arg(r).arg(g).arg(b);
						i += 4;
					}
					break;
				case 48:
					if (code == 48 && params.size() >= 5 && params[1] == "2" || i == 0) {
						int r = params[2].toInt();
						int g = params[3].toInt();
						int b = params[4].toInt();
						styleString += QString("background-color: rgb(%1, %2, %3);").arg(r).arg(g).arg(b);
						i += 4;
					}
					break;
				case 0:
					styleString += "font-weight: normal; font-style: normal; text-decoration: none; color: initial; background-color: initial;";
					break;
				case 1:
					styleString += "font-weight: bold;";
					break;
				case 3:
					styleString += "font-style: italic;";
					break;
				case 4:
					styleString += "text-decoration: underline;";
					break;
				case 5:
					// Blink is not supported in HTML
					break;
				case 7:
					styleString += "filter: invert(100%);";
					break;
				case 9:
					styleString += "text-decoration: line-through;";
					break;
				case 30:
					styleString += "color: black;";
					break;
				case 31:
					styleString += "color: red;";
					break;
				case 32:
					styleString += "color: green;";
					break;
				case 33:
					styleString += "color: yellow;";
					break;
				case 34:
					styleString += "color: blue;";
					break;
				case 35:
					styleString += "color: purple;";
					break;
				case 36:
					styleString += "color: cyan;";
					break;
				case 37:
					styleString += "color: lightgray;";
					break;
				case 90:
					styleString += "color: gray;";
					break;
				case 91:
					styleString += "color: lightred;";
					break;
				case 92:
					styleString += "color: lightgreen;";
					break;
				case 93:
					styleString += "color: lightyellow;";
					break;
				case 94:
					styleString += "color: lightblue;";
					break;
				case 95:
					styleString += "color: lightpurple;";
					break;
				case 96:
					styleString += "color: lightcyan;";
					break;
				case 97:
					styleString += "color: white;";
					break;
				default:
					break;
				}
			}
			styleString += "\">";
			htmlString.replace(match.captured(0), styleString);
		}
		return htmlString;
	}

	/*!
		\since Visindigo 0.13.0
		\a msg 为要输出的文本。

		此输出自动换行。
	*/
	void Console::print(QString msg) {
		qt_message_output(QtDebugMsg, QMessageLogContext(), msg);
	}

	/*!
		\since Visindigo 0.13.0
		\a data 为要进行二进制显示的数据。

		一般来说，这个函数在控制台上的输出如下所示：
		\code
			L\B	00  01  02  03  04  05  06  07  |  00 01 02 03 04 05 06 07  |
			---------------------------------------------------------------------
			0	00  01  02  03  04  05  06  07  |  . . . . . . . .  |
			1	08  09  0A  0B  0C  0D  0E  0F  |  . . . . . . . .  |
		\endcode
		\note 如果数据中存在未初始化的内存，将会在输出的最后一行显示警告。
	*/
	void Console::binary(const QByteArray& data) {
		print(binaryToString(data));
	}
	
	QString Console::binaryToString(const QByteArray& data) {
		if (data.isEmpty()) { return QString(); }
		QString result;
		result.append("L\\B\t00  01  02  03  04  05  06  07  |  00 01 02 03 04 05 06 07  |\n");
		result.append("---------------------------------------------------------------------\n");
		int B = 0;
		int L = 0;
		QString rawText = "";
		QString chatText = "";
		bool UninitializedMemory = false;
		for (auto i = data.begin(); i != data.end(); i++) {
			if ((unsigned char)(*i) < (unsigned char)0x00 || (unsigned char)(*i) > (unsigned char)0xFF) {
				rawText.append("??  ");
				UninitializedMemory = true;
			}
			else {
				rawText.append(QString::number((unsigned char)(*i), 16).rightJustified(2, '0').toUpper() + "  ");
			}
			if (*i >= 0x20 && *i <= 0x7E) {
				chatText.append(" " % QChar(*i) % " ");
			}
			else if (*i == 0x0A) {
				chatText.append("\\n ");
			}
			else if (*i == 0x0D) {
				chatText.append("\\r ");
			}
			else if (*i == 0x09) {
				chatText.append("\\t ");
			}
			else {
				chatText.append(" . ");
			}
			B++;
			L++;
			if (B == 8) {
				result.append(QString::number(L, 10) % "\t" % rawText % "|  " % chatText % " |\n");
				rawText = "";
				chatText = "";
				B = 0;
			}
		}
		if (B != 0) {
			for (int i = 0; i < 8 - B; i++) {
				rawText.append("    ");
				chatText.append("   ");
			}
			result.append(QString::number(L, 10) % "\t" % rawText % "|  " % chatText % " |");
		}
		result.append("\n---------------------------------------------------------------------\n");
		if (UninitializedMemory) {
			result.append("Warning: Uninitialized memory detected.(Part marked as '??')\nIf this data directly comes from a struct or class, please ignore this issue");
		}
		return result;
	}

	/*! \fn template <typename T> static void Console::memory(const T * data)
		\since Visindigo 0.13.0
		\a data 为要进行二进制显示的对象

		此函数本质上可看做binary()的某种意义上的重载。
	*/

	/*!
		\since Visindigo 0.13.0
		\a cmd 为要执行的命令。

		执行一个命令。
		\note 这个函数直接要求系统执行命令，所以具有平台相关性。
	*/
	unsigned int Console::exec(const QString& cmd) {
		return system(cmd.toLocal8Bit().data());
	}
}