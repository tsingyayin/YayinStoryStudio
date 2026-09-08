#ifndef Visindigo_General_private_LoggerMsgHandler_impl_h
#define Visindigo_General_private_LoggerMsgHandler_impl_h
#include <type_traits>
#include <typeinfo>
#include <QtCore/qmetaobject.h>
#include <QtCore/qstringlist.h>

namespace Visindigo::General {
	// Q_ENUM 注册的单值枚举：打印 QMetaEnum 的键名；未识别的值退化为十进制数值。
	template<EnumHasQEnum T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(T enumValue) {
		const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
		const char* key = metaEnum.valueToKey(static_cast<int>(enumValue));
		fromString((key != nullptr) ? QString::fromLatin1(key) : QString::number(static_cast<qint64>(enumValue)));
		return *this;
	}
	// Q_FLAG 位域组合：打印 QMetaEnum 的键名组合（如 "FlagA|FlagB"）；无法全部识别时退化为十六进制。
	template<EnumHasQFlag T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QFlags<T> enumFlags) {
		const QMetaEnum metaEnum = QMetaEnum::fromType<QFlags<T>>();
		const QByteArray keys = metaEnum.valueToKeys(enumFlags.toInt());
		fromString(keys.isEmpty()
			? QStringLiteral("0x") % QString::number(static_cast<qint64>(enumFlags.toInt()), 16).toUpper()
			: QString::fromLatin1(keys));
		return *this;
	}
	// 未注册的普通枚举：无 Qt 元对象可取键名，退化为 (类型名) 十进制 (0x十六进制)。
	template<EnumWithoutMeta T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(T enumValue) {
		const qint64 value = static_cast<qint64>(enumValue);
		fromString(QStringLiteral("(%1) %2 (0x%3)")
			.arg(QString::fromLatin1(typeid(T).name()))
			.arg(value)
			.arg(QString::number(static_cast<quint64>(value), 16)));
		return *this;
	}
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(T t) {
		fromString(t.toString());
		return *this;
	}
	// 任意 T 的 QMap：无专门重载时打印“对象地址”兜底。
	template<typename T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> any_map) {
		QString temp = QStringLiteral("QMap{");
		for (auto it = any_map.constBegin(); it != any_map.constEnd(); ++it) {
			temp += QStringLiteral(" \"") % it.key() % QStringLiteral("\": [Object at ") %
				QString::number(static_cast<quint64>(reinterpret_cast<quintptr>(&it.value())), 16) % QStringLiteral("],");
		}
		if (not any_map.isEmpty()) {
			temp.chop(1); // 去掉末尾逗号
		}
		temp += QStringLiteral(" }");
		fromString(temp);
		return *this;
	}
	// Printable 值 QMap。
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> printable_map) {
		QString temp = QStringLiteral("QMap{");
		for (auto it = printable_map.constBegin(); it != printable_map.constEnd(); ++it) {
			temp += QStringLiteral(" \"") % it.key() % QStringLiteral("\": ") % it.value().toString() % QStringLiteral(",");
		}
		if (not printable_map.isEmpty()) {
			temp.chop(1);
		}
		temp += QStringLiteral(" }");
		fromString(temp);
		return *this;
	}
	// 任意 T 的 QList：无专门重载时打印“对象地址”兜底。
	template<typename T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> list) {
		QString temp = QStringLiteral("QList[");
		for (int i = 0; i < list.size(); ++i) {
			temp += QStringLiteral(" [Object at ") %
				QString::number(static_cast<quint64>(reinterpret_cast<quintptr>(&list[i])), 16) % QStringLiteral("],");
		}
		if (not list.isEmpty()) {
			temp.chop(1);
		}
		temp += QStringLiteral(" ]");
		fromString(temp);
		return *this;
	}
	// Printable 值 QList。
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> list) {
		QString temp = QStringLiteral("QList[");
		for (int i = 0; i < list.size(); ++i) {
			temp += QStringLiteral(" ") % list[i].toString() % QStringLiteral(",");
		}
		if (not list.isEmpty()) {
			temp.chop(1);
		}
		temp += QStringLiteral(" ]");
		fromString(temp);
		return *this;
	}
}
#endif // Visindigo_General_private_LoggerMsgHandler_impl_h
