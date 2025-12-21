#include "General/LogMetaData.h"
#include <QtCore/qstring.h>

namespace Visindigo::General {
	class LogMetaDataPrivate {
		friend class LogMetaData;
	protected:
		QString FunctionName;
		qint32 LineNumber;
	};

	/*!
		\class Visindigo::General::LogMetaData
		\inheaderfile General/LogMetaData.h
		\brief 日志元数据类，用于存储日志消息的附加信息。
		\inmodule Visindigo
		\since Visindigo 0.13.0
		\ingroup VDebug

		LogMetaData 类用于存储与日志消息相关的附加信息，例如函数名称和行号。
		这些信息可以帮助开发者更好地定位日志消息的来源，从而更有效地进行调试和问题排查。

		因为这个类输入数据所依赖的__FUNCTION__宏和__LINE__宏在绝大多数
		现代编译器下都可用，因此该类的信息几乎总是有效的，但不同编译器在__FUNCTION__宏
		的实现细节上有所差异，例如MSVC下返回的是完整的函数名，而GCC和Clang下
		则只有函数名本身，不带有命名空间和类名等信息。

		如果需要完整函数签名，要考虑Q_FUNC_INFO宏。

		如果需要更详细的堆栈跟踪信息，请使用Visindigo::General::StacktraceHelper类。

		在VDebug中，这两个类一般同时使用，即使部分信息是重复的。

		与Visindigo::General::StacktraceFrame类不同，LogMetaData类是可变的，因此可以在创建后修改其内容。
	*/

	/*!
		\since Visindigo 0.13.0
		默认构造函数，创建一个空的日志元数据对象。
	*/
	LogMetaData::LogMetaData() {
		d = new LogMetaDataPrivate;
		d->FunctionName = "";
		d->LineNumber = -1;
	}

	/*!
		\since Visindigo 0.13.0
		构造函数，使用指定的函数名称和行号创建一个日志元数据对象。

		\a funcName 函数名称。
		\a lineNumber 行号。
	*/
	LogMetaData::LogMetaData(const QString& funcName, qint32 lineNumber) {
		d = new LogMetaDataPrivate;
		d->FunctionName = funcName;
		d->LineNumber = lineNumber;
	}

	/*!
		\fn Visindigo::General::LogMetaData::LogMetaData(const LogMetaData& other)
		\since Visindigo 0.13.0

		拷贝构造函数
	*/

	/*!
		\fn Visindigo::General::LogMetaData::LogMetaData(LogMetaData&& other) noexcept
		\since Visindigo 0.13.0

		移动构造函数
	*/

	/*!
		\fn Visindigo::General::LogMetaData& Visindigo::General::LogMetaData::operator=(const LogMetaData& other)
		\since Visindigo 0.13.0

		拷贝赋值运算符
	*/

	/*!
		\fn Visindigo::General::LogMetaData& Visindigo::General::LogMetaData::operator=(LogMetaData&& other) noexcept
		\since Visindigo 0.13.0

		移动赋值运算符
	*/

	VIMoveable_Impl(LogMetaData);
	VICopyable_Impl(LogMetaData);

	/*!
		\since Visindigo 0.13.0
		析构函数，销毁日志元数据对象。
	*/
	LogMetaData::~LogMetaData() {
		if (d != nullptr) {
			delete d;
		}
	}

	/*!
		\since Visindigo 0.13.0
		return 函数名称
	*/
	QString LogMetaData::getFunctionName() const {
		return d->FunctionName;
	}

	/*!
		\since Visindigo 0.13.0
		return 行号
	*/
	qint32 LogMetaData::getLineNumber() const {
		return d->LineNumber;
	}

	/*!
		\since Visindigo 0.13.0
		\a funcName 函数名称
		设置函数名称
	*/
	void LogMetaData::setFunctionName(const QString& funcName) {
		d->FunctionName = funcName;
	}

	/*!
		\since Visindigo 0.13.0
		\a lineNumber 行号
		设置行号
	*/
	void LogMetaData::setLineNumber(qint32 lineNumber) {
		d->LineNumber = lineNumber;
	}

	/*!
		\since Visindigo 0.13.0
		return 日志元数据是否有效

		此类将函数名称为空且行号为-1的对象视为无效对象。
		如果需要表达“函数有效，但行号未知”，请将行号设置为0。

		这在某些情况下很有用。
	*/
	bool LogMetaData::isValid() const {
		return !(d->FunctionName.isEmpty() && d->LineNumber == -1);
	}

	/*!
		\since Visindigo 0.13.0
		return 将日志元数据格式化为字符串表示形式。

		这是个用于调试的辅助函数，Visindigo输出日志时不依赖此函数，因此调用此函数
		输出的结果和在日志上看到的结果有所不同。
	*/
	QString LogMetaData::toString() const {
		return QString("%1 : %2").arg(d->FunctionName).arg(d->LineNumber);
	}

}