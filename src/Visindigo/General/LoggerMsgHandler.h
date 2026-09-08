#ifndef Visindigo_General_LoggerMsgHandler_h
#define Visindigo_General_LoggerMsgHandler_h
#include <QtCore/qstring.h>
#include <QtCore/qtypes.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include "VICompileMacro.h"
#include "General/Logger.h"
#include "General/LogMetaData.h"
#include "General/StacktraceHelper.h"
#include <QtCore/qmetaobject.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qdebug.h>

// Forward declarations
class QSize;
class QRect;
namespace Visindigo::General {
	template <typename T> concept Printable = requires(T t) {
		{ t.toString() }->::std::same_as<QString>;
	};
	// ADL 探测：qt_getEnumMetaObject 由 Qt moc 为 Q_ENUM / Q_FLAG 生成（是非模板 ADL 函数），
	// 因此必须“不带显式模板实参”地调用，让 ADL 在枚举所属命名空间解析到它。
	// EnumHasQFlag：该枚举的 QFlags<T> 具备元对象，即声明为 Q_FLAG 的位域枚举。
	template<typename T> concept EnumHasQFlag = requires(T t, QFlags<T> enumFlags) {
		requires std::is_enum_v<T>;
		{ qt_getEnumMetaObject(enumFlags) } -> std::same_as<const QMetaObject*>;
	};
	// EnumHasQEnum：该枚举自身具备元对象，即用 Q_ENUM 注册的枚举。
	template<typename T> concept EnumHasQEnum = requires(T t) {
		requires std::is_enum_v<T>;
		{ qt_getEnumMetaObject(t) } -> std::same_as<const QMetaObject*>;
	};
	// EnumWithoutMeta：普通枚举（没有 Q_ENUM / Q_FLAG 元对象）。
	// 注：为避免与“已注册枚举”的重载歧义，这里显式排除有元对象的枚举。
	template<typename T> concept EnumWithoutMeta = requires {
		requires std::is_enum_v<T>;
		requires !EnumHasQEnum<T>;
		requires !EnumHasQFlag<T>;
	};
}
namespace Visindigo::General {
	// Main
	class VisindigoAPI LoggerMsgHandler final {
		friend class Logger;
	private:
		Logger* Who;
		QStringList Msg;
		QString JoinedCache;   // getMessage() 的 join 缓存：首次拼接后复用
		bool JoinedDirty = true; // Msg 有新片段时置真，getMessage 下次重建缓存
		Logger::Level Level;
		LogMetaData MetaData;
		QList<StacktraceFrame> Stacktrace;
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
		template<EnumHasQEnum T> LoggerMsgHandler& operator<<(T enumValue); // Q_ENUM 注册的枚举：打印键名
		template<EnumHasQFlag T> LoggerMsgHandler& operator<<(QFlags<T> enumFlags); // Q_FLAG 位域组合：打印键名组合
		template<EnumWithoutMeta T> LoggerMsgHandler& operator<<(T enumValue); // 未注册普通枚举：(类型名) 十进制 (十六进制)

		template<typename T> LoggerMsgHandler& operator<<(QMap<QString, T> any_map);

		LoggerMsgHandler& operator<<(QMap<QString, QObject*> pointer_map);
		LoggerMsgHandler& operator<<(const QMap<QString, QString>& string_map);
		template<Printable T>LoggerMsgHandler& operator<<(QMap<QString, T> printable_map); // for any map with printable values

		template<typename T>LoggerMsgHandler& operator<<(QList<T> any_list);
		LoggerMsgHandler& operator<<(QList<qint64> num_list);
		LoggerMsgHandler& operator<<(QList<QObject*> qobject_list);
		template<Printable T>LoggerMsgHandler& operator<<(QList<T> qobject_list); // for any list with printable values

		LoggerMsgHandler& operator<<(const QSize& size);
		LoggerMsgHandler& operator<<(const QRect& rect);

		QString getMessage();
		Logger* getLogger();
		Logger::Level getLevel();
		LogMetaData getMetaData();
		QList<StacktraceFrame> getStacktrace();
	};
}

#include "private/LoggerMsgHandler_impl.hpp"

#endif // Visindigo_General_LoggerMsgHandler_h