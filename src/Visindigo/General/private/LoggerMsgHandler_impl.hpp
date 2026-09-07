#ifndef Visindigo_General_private_LoggerMsgHandler_impl_h
#define Visindigo_General_private_LoggerMsgHandler_impl_h
#include <type_traits>

namespace Visindigo::General {
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(T t) {
		LogUnits.append(Utility::ConsoleFormat(t.toString()));
		return *this;
	}
	template<typename T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> pointer_map) {
		QStringList parts;
		parts.reserve(pointer_map.size()+2);
		parts << QStringLiteral("QMap{");
		for (auto it = pointer_map.constBegin(); it != pointer_map.constEnd(); ++it) {
			parts << QStringLiteral("\"") % it.key() % QStringLiteral("\": [Object at ") % QString::number((quint64)(T*)(&it.value()), 16) % QStringLiteral("],");
		}
		if (not pointer_map.isEmpty()) {
			parts.last().removeLast();
		}
		parts << QStringLiteral("}");
		LogUnits.append(Utility::ConsoleFormat(parts.join("")));
		return *this;
	}
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> pointer_map) {
		QStringList parts;
		parts.reserve(pointer_map.size() + 2);
		parts << QStringLiteral("QMap{");
		for (const QString& key : pointer_map.keys()) {
			parts << QStringLiteral(" \"") % key % QStringLiteral("\": ") % pointer_map.value(key).toString() % QStringLiteral(",");
		}
		if (not pointer_map.isEmpty()) {
			parts.last().removeLast();
		}
		parts << QStringLiteral(" }");
		LogUnits.append(Utility::ConsoleFormat(parts.join("")));
		return *this;
	}
	template<typename T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> list) {
		QStringList parts;
		parts.reserve(list.size() + 2);
		parts << QStringLiteral("QList[");
		for (const T& item : list) {
			parts << QStringLiteral(" [Object at ") % QString::number((quint64)&item, 16) % QStringLiteral("],");
		}
		if (not list.isEmpty()) {
			parts.last().removeLast();
		}
		parts << QStringLiteral(" ]");
		LogUnits.append(Utility::ConsoleFormat(parts.join("")));
		return *this;
	}
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> list) {
		QStringList parts;
		parts.reserve(list.size() + 2);
		parts << QStringLiteral("QList[");
		for (const T& item : list) {
			parts << QStringLiteral(" ") % item.toString() % QStringLiteral(",");
		}
		if (not list.isEmpty()) {
			parts.last().removeLast();
		}
		parts << QStringLiteral(" ]");
		LogUnits.append(Utility::ConsoleFormat(parts.join("")));
		return *this;
	}
}
#endif // Visindigo_General_private_LoggerMsgHandler_impl_h
