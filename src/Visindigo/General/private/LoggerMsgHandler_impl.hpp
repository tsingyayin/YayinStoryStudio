#ifndef Visindigo_General_private_LoggerMsgHandler_impl_h
#define Visindigo_General_private_LoggerMsgHandler_impl_h
#include <type_traits>
#include <typeinfo>
#include <QtCore/qmetaenum.h>

namespace Visindigo::General {
	// Q_ENUM 注册的单值枚举：打印 QMetaEnum 的键名；未识别的值退化为十进制数值。
	template<EnumHasQEnum T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(T enumValue) {
		QString text;
		const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
		const char* key = metaEnum.valueToKey(static_cast<int>(enumValue));
		text = (key != nullptr) ? QString::fromLatin1(key) : QString::number(static_cast<qint64>(enumValue));
		Msg += text;
		return *this;
	}
	// Q_FLAG 位域组合：打印 QMetaEnum 的键名组合（如 "FlagA|FlagB"）；无法全部识别时退化为十六进制。
	template<EnumHasQFlag T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QFlags<T> enumFlags) {
		QString text;
		const QMetaEnum metaEnum = QMetaEnum::fromType<QFlags<T>>();
		const QByteArray keys = metaEnum.valueToKeys(enumFlags.toInt());
		text = keys.isEmpty()
			? QStringLiteral("0x") % QString::number(static_cast<qint64>(enumFlags.toInt()), 16).toUpper()
			: QString::fromLatin1(keys);
		Msg += text;
		return *this;
	}
	// 未注册的普通枚举：无 Qt 元对象可取键名，退化为 (类型名) 十进制 (0x十六进制)。
	template<EnumWithoutMeta T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(T enumValue) {
		const qint64 value = static_cast<qint64>(enumValue);
		const QString text = QStringLiteral("(%1) %2 (0x%3)")
			.arg(QString::fromLatin1(typeid(T).name()))
			.arg(value)
			.arg(QString::number(static_cast<quint64>(value), 16));
		Msg += text;
		return *this;
	}
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(T t) {
		Msg += t.toString();
		return *this;
	}
	template<typename T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> pointer_map) {
		QString temp = "QMap{";
		for (QString key : pointer_map) {
			void* ptr = &pointer_map[key];
			temp += " \"" % key % "\": [Object at " % QString::number((quint64)ptr, 16) % "],";
		}
		temp.removeLast();
		temp += "}";
		Msg += temp;
		return *this;
	}
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> pointer_map) {
		QString temp = "QMap{";
		for (QString key : pointer_map) {
			temp += " \"" % key % "\": " % pointer_map[key].toString() % ",";
		}
		temp.removeLast();
		temp += "}";
		Msg += temp;
		return *this;
	}
	template<typename T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> list) {
		QString temp = "QList[";
		for (int i = 0; i < list.size(); i++) {
			void* ptr = &list[i];
			temp += " [Object at " % QString::number((quint64)ptr, 16) % "],";
		}
		temp.removeLast();
		temp += "}";
		Msg += temp;
		return *this;
	}
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> list) {
		QString temp = "QList[";
		for (int i = 0; i < list.size(); i++) {
			temp += " " % list[i].toString() % ",";
		}
		temp.removeLast();
		temp += "}";
		Msg += temp;
		return *this;
	}
}
#endif // Visindigo_General_private_LoggerMsgHandler_impl_h
