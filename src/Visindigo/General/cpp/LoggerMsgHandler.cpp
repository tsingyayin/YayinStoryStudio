#include "../LoggerMsgHandler.h"
#include "../LoggerManager.h"
#include "../private/LoggerMsgHandler_p.h"
#include <QtCore/qobject.h>
#include "../../Utility/Console.h"

namespace Visindigo::General {
	/*!
		\class Visindigo::General::LoggerMsgHandler
		\inheaderfile General/LoggerMsgHandler.h
		\inmodule Visindigo
		\brief LoggerMsgHandler是日志消息处理器，用于承接日志消息并将其传递给LoggerManager进行处理。
		\since Visindigo 0.13.0
		\ingroup VDebug
		\inmodule Visindigo

		LoggerMsgHandler类用于承接日志消息，并在析构时将其传递给LoggerManager进行处理。
		这个设计是因为，LoggerMsgHandler的大部分使用场景下，应该被创建为一个纯右值（或叫它将亡值），
		即一行内没有将被赋给左值的临时对象。因此，我们可以利用其析构函数在对象生命周期结束时
		自动将日志消息传递给LoggerManager进行处理。如果用户有什么延迟输出日志的需求，可以
		自行保存到左值中并在适当的时候销毁它。（不推荐这么做，但可能会很有意思。）

		用户无需直接创建LoggerMsgHandler对象，而是通过Logger类提供的宏来间接创建它们。

		此类利用模板实现了大多数常用数据类型的日志消息承接功能，包括基本数据类型、QString、
		符合一定要求的QList和QMap等。用户也可以通过继承Printable概念来实现自定义类型的日志消息承接功能。

		\section1 概念 Printable 
		VDebug使用了C++20提供的concept特性定义了一个Printable概念，任何类型只要实现了toString()方法并返回
		QString类型的结果，就可以被视为符合Printable概念的类型，从而可以被LoggerMsgHandler承接。

		Printable不在LoggerMsgHandler类内定义，这是个处于Visindigo::General命名空间内的独立概念，用户可以在任何需要的地方使用它来约束类型。
		它定义如下：
		\code
		template <typename T> concept Printable = requires(T t) {
			{ t.toString() }->::std::same_as<QString>;
		};
		\endcode
	*/

	/*!
		\since Visindigo 0.13.0
		构造LoggerMsgHandler实例。
		\a who 指向创建此LoggerMsgHandler对象的Logger对象的指针。
		\a level 日志级别。

		此构造函数由Logger类的日志宏间接调用，用户无需直接使用它。
	*/
	LoggerMsgHandler::LoggerMsgHandler(Logger* who, Logger::Level level) {
		d = new Visindigo::__Private__::LoggerMsgHandlerPrivate;
		d->Logger = who;
		d->Level = level;
	}

	/*!
		\since Visindigo 0.13.0
		析构函数

		此析构函数会在LoggerMsgHandler对象销毁时自动调用，用于将日志消息传递给LoggerManager进行处理。
	*/
	LoggerMsgHandler::~LoggerMsgHandler() {
		LoggerManager::getInstance()->msgHandlerLog(this);
	}

	/*!
		\since Visindigo 0.13.0
		将字符串追加到日志消息中。
		\a str 要追加的字符串。

		此函数用于将字符串追加到日志消息中，用户可以通过重载的<<运算符间接调用它。
	*/
	void LoggerMsgHandler::fromString(const QString& str) {
		d->Msg += str % " ";
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示QString类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const QString& str) {
		fromString("\"" % str % "\"");
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示float类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(float num) {
		fromString(QString::number(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示double类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(double num) {
		fromString(QString::number(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示qint8类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(qint8 num) {
		fromString(QChar(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示quint8类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(quint8 num) {
		fromString(QString::number(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示qint16类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(qint16 num) {
		fromString(QString::number(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示quint16类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(quint16 num) {
		fromString(QString::number((quint32)num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示qint32类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(qint32 num) {
		fromString(QString::number(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示quint32类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(quint32 num) {
		fromString(QString::number(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示qint64类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(qint64 num) {
		fromString(QString::number(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示quint64类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(quint64 num) {
		fromString(QString::number(num));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示bool类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(bool b) {
		fromString(b ? "true" : "false");
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示const char*类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const char* str) {
		fromString(QString::fromUtf8(str));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示QStringList类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const QStringList& strList) {
		fromString(QString("[%1]").arg(strList.join(", ")));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示QByteArray类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const QByteArray& byteArray) {
		fromString("\n"+Visindigo::Utility::Console::binaryToString(byteArray));
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示QObject*类型的实现。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QObject* pointer) {
		if (pointer == nullptr) {
			fromString("nullptr");
		}
		else {
			fromString(QString("%1(%2)").arg(pointer->metaObject()->className()).arg((quint64)(void*)pointer, 0, 16));
		}
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		允许从<<运算符输入日志的元信息。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const LogMetaData& metaData) {
		d->MetaData = metaData;
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		允许从<<运算符输入日志的堆栈跟踪信息。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const QList<StacktraceFrame>& stacktrace) {
		d->Stacktrace = stacktrace;
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示QMap的key为QString，value为QObject*指针类型的实现。
		它输出每个键值对的key和value的类名及地址。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, QObject*> pointer_map) {
		QStringList list;
		for (auto it = pointer_map.begin(); it != pointer_map.end(); ++it) {
			if (it.value() == nullptr) {
				list.append(it.key() % ": nullptr");
			}
			else {
				list.append(it.key() % ": " % it.value()->metaObject()->className() % "(" % QString::number((quint64)(void*)it.value(), 16) % ")");
			}
		}
		fromString("{" % list.join(", ") % "}");
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		重载<<运算符以承接各种类型的日志消息。这里展示成员为QObject*指针类型的QList的实现。
		它输出每个QObject指针的类名及地址。
	*/
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<QObject*> qobject_list) {
		QStringList list;
		for (QObject* obj : qobject_list) {
			if (obj == nullptr) {
				list.append("nullptr");
			}
			else {
				list.append(QString(obj->metaObject()->className()) % "(" % QString::number((quint64)(void*)obj, 16) % ")");
			}
		}
		fromString("[" % list.join(", ") % "]");
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		获取日志消息内容。
		\return 返回日志消息内容的QString表示。

		此函数用于获取当前LoggerMsgHandler对象承接的日志消息内容。
	*/
	QString LoggerMsgHandler::getMessage() {
		return d->Msg;
	}

	/*!
		\since Visindigo 0.13.0
		获取创建此LoggerMsgHandler对象的Logger对象的指针。
		\return 返回指向Logger对象的指针。

		此函数用于获取创建此LoggerMsgHandler对象的Logger对象的指针。
	*/
	Logger* LoggerMsgHandler::getLogger() {
		return d->Logger;
	}

	/*!
		\since Visindigo 0.13.0
		获取日志级别。
		\return 返回日志级别枚举值。

		此函数用于获取当前LoggerMsgHandler对象的日志级别。
	*/
	Logger::Level LoggerMsgHandler::getLevel() {
		return d->Level;
	}

	/*!
		\since Visindigo 0.13.0
		获取日志元信息。
		\return 返回LogMetaData对象。
		此函数用于获取当前LoggerMsgHandler对象的日志元信息。
	*/
	LogMetaData LoggerMsgHandler::getMetaData() {
		return d->MetaData;
	}

	/*!
		\since Visindigo 0.13.0
		获取日志堆栈跟踪信息。
		\return 返回包含StacktraceFrame对象的QList。
		此函数用于获取当前LoggerMsgHandler对象的日志堆栈跟踪信息。
	*/
	QList<StacktraceFrame> LoggerMsgHandler::getStacktrace() {
		return d->Stacktrace;
	}

	/*!
		\fn template<Printable T> LoggerMsgHandler& LoggerMsgHandler::operator<<(T type)
		\since Visindigo 0.13.0
		
		重载<<运算符以承接符合Printable概念的自定义类型的日志消息。
	*/

	/*!
		\fn template<typename T> LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> any_map)
		\since Visindigo 0.13.0
		重载<<运算符以承接成员为任意类型的QMap日志消息。
		这里强制将每个成员输出为其内存地址。
		typename T 为 Printable 时，则输出toString()的结果。
	*/

	/*!
		\fn template<typename T> LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> any_list)
		\since Visindigo 0.13.0
		重载<<运算符以承接成员为任意类型的QList日志消息。
		这里强制将每个成员输出为其内存地址。
		typename T 为 Printable 时，则输出toString()的结果。
	*/
}