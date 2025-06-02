#pragma once
#include "../Macro.h"
#include "Logger.h"
#include <QtCore/qstring.h>
#include  <QtCore/qtypes.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <type_traits>
#include "private/LoggerMsgHandler_p.h"

namespace YSSCore::General {
	template <typename T>
	concept Printable = requires(T t) {
		{ t.toString() }->::std::same_as<QString>;
	};

	class YSSCoreAPI LoggerMsgHandler final {
		friend class Logger;
	protected:
		LoggerMsgHandler(Logger* who, Logger::Level level);
	public:
		LoggerMsgHandler(const LoggerMsgHandler& other) = delete;
		LoggerMsgHandler(LoggerMsgHandler&& other) = delete;
		LoggerMsgHandler& operator=(const LoggerMsgHandler& other) = delete;
		LoggerMsgHandler& operator=(LoggerMsgHandler&& other) = delete;
		~LoggerMsgHandler();
		void fromString(const QString& str);
		LoggerMsgHandler& operator<<(const QString& str);
		LoggerMsgHandler& operator<<(float num);
		LoggerMsgHandler& operator<<(double num);
		LoggerMsgHandler& operator<<(qint8 chr);
		LoggerMsgHandler& operator<<(quint8 chr);
		LoggerMsgHandler& operator<<(qint16 num);
		LoggerMsgHandler& operator<<(quint16 num);
		LoggerMsgHandler& operator<<(qint32 num);
		LoggerMsgHandler& operator<<(quint32 num);
		LoggerMsgHandler& operator<<(qint64 num);
		LoggerMsgHandler& operator<<(quint64 num);
		LoggerMsgHandler& operator<<(bool b);
		LoggerMsgHandler& operator<<(const char* str);
		LoggerMsgHandler& operator<<(const QStringList& strList);
		template<Printable T> LoggerMsgHandler& operator<<(T type);
		template<typename T>LoggerMsgHandler& operator<<(QMap<QString, T> pointer_map);
		LoggerMsgHandler& operator<<(QMap<QString, QObject*> pointer_map);
		template<Printable T>LoggerMsgHandler& operator<<(QMap<QString, T> printable_map);
		template<typename T>LoggerMsgHandler& operator<<(QList<T> list);
		LoggerMsgHandler& operator<<(QList<QObject*> qobject_list);
		template<Printable T>LoggerMsgHandler& operator<<(QList<T> qobject_list);
		QString getMessage();
		Logger* getLogger();
		Logger::Level getLevel();
	private:
		YSSCore::__Private__::LoggerMsgHandlerPrivate* d;
	};
}

#include "private/LoggerMsgHandler_impl.hpp"