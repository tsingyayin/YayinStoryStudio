#pragma once
#include <QtCore/qstring.h>
#include <QtCore/qtypes.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include "../VICompileMacro.h"
#include "Logger.h"
#include "private/LoggerMsgHandler_p.h"
#include "LogMetaData.h"
#include "StacktraceHelper.h"
#include <QtCore/qmetaobject.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qdebug.h>
// Forward declarations
class QSize;
class QRect;
namespace Visindigo::__Private__ {
	class LoggerMsgHandlerDataPool;
	class LoggerMsgHandlerPrivate;
}
namespace Visindigo::General {
	template <typename T> concept Printable = requires(T t) {
		{ t.toString() }->::std::same_as<QString>;
	};
	//template<typename ADLFallback> void qt_getEnumMetaObject(const ADLFallback&) {}
	template<typename T> concept EnumHasQFlag = requires(T t) {
		requires std::is_enum_v<T>;
		requires std::is_same_v<const QMetaObject*, decltype(qt_getEnumMetaObject<QFlags<T>>(t))>;
	};
	template<typename T> concept EnumHasQEnum = requires(T t) {
		requires std::is_enum_v<T>;
		requires !std::is_same_v<const QMetaObject*, decltype(qt_getEnumMetaObject<QFlags<T>>(t))>;
		requires std::is_same_v<const QMetaObject*, decltype(qt_getEnumMetaObject<T>(t))>;
	};
}
namespace Visindigo::General {
	// Main
	class VisindigoAPI LoggerMsgHandler final {
		friend class Logger;
		friend class Visindigo::__Private__::LoggerMsgHandlerDataPool;
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
		LoggerMsgHandler& operator<<(const QByteArray& byteArray);
		LoggerMsgHandler& operator<<(QObject* pointer);
		LoggerMsgHandler& operator<<(const LogMetaData& metaData);
		LoggerMsgHandler& operator<<(const QList<StacktraceFrame>& stacktrace);

		template<Printable T> LoggerMsgHandler& operator<<(T type); // for any type with toString() method

		template<typename T> LoggerMsgHandler& operator<<(QMap<QString, T> any_map);

		LoggerMsgHandler& operator<<(QMap<QString, QObject*> pointer_map);
		LoggerMsgHandler& operator<<(QMap<QString, QString> string_map);
		template<Printable T>LoggerMsgHandler& operator<<(QMap<QString, T> printable_map); // for any map with printable values

		template<typename T>LoggerMsgHandler& operator<<(QList<T> any_list);
		LoggerMsgHandler& operator<<(QList<QObject*> qobject_list);
		template<Printable T>LoggerMsgHandler& operator<<(QList<T> qobject_list); // for any list with printable values

		LoggerMsgHandler& operator<<(const QSize& size);
		LoggerMsgHandler& operator<<(const QRect& rect);

		QString getMessage();
		Logger* getLogger();
		Logger::Level getLevel();
		LogMetaData getMetaData();
		QList<StacktraceFrame> getStacktrace();
	protected:
		Visindigo::__Private__::LoggerMsgHandlerPrivate* d;
	};
	template<typename T>
	void testTemplate(T t) {
		// do nothing, just for testing concepts
	}

	template<EnumHasQEnum T>
	void testTemplate(T t) {
		static_assert(false, "qenum detected");
	}

	template<EnumHasQFlag T>
	void testTemplate(T t) {
		static_assert(false, "qflag detected");
	}

}

#include "private/LoggerMsgHandler_impl.hpp"