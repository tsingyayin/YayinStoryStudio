#include "General/Exception.h"
namespace Visindigo::General {

	class ExceptionPrivate {
		public:
		Exception::Type Type;
		QString Message;
		bool Critical;
		QString File;
		int Line;
		QString Function;
		QList<StacktraceFrame> Stacktrace;
	};

	/*!
		\class Visindigo::General::Exception
		\inheaderfile General/Exception.h
		\brief 异常类，用于表示和处理程序中的异常情况。
		\inmodule Visindigo
		\since Visindigo 0.13.0

		Exception 类用于表示程序中的异常情况。它包含异常的类型、消息、严重性、
		发生位置（文件名、行号、函数名）以及堆栈跟踪信息。

		这类与std::exception无关，但可以通过fromStdException函数将std::exception转换为
		此类的实例。此类也同样实现了std风格的what()函数，以便与标准异常处理机制兼容。

		这类是不可变的，一旦创建，其内容就不能更改。
	*/

	/*!
		\enum Visindigo::General::Exception::Type
		\since Visindigo 0.13.0
		\value Unknown 未知异常类型，值为0x0000000
		\value Other 其他异常类型
		\value IOError 输入输出异常类型
		\value ParseError 解析异常类型
		\value InvalidArgument 无效参数异常类型
		\value OutOfRange 越界异常类型
		\value NotFound 未找到异常类型
		\value AlreadyExists 已存在异常类型
		\value PermissionDenied 权限拒绝异常类型
		\value Timeout 超时异常类型
		\value UnsupportedOperation 不支持的操作异常类型
		\value InternalError 内部错误异常类型
		\value OutOfMemory 内存不足异常类型
		\value Std_Unknown 标准未知异常类型，值为0x8000000
		\value Std_Other 标准其他异常类型
		\value Std_LogicError std::logic_error
		\value Std_InvalidArgument std::invalid_argument
		\value Std_DomainError std::domain_error
		\value Std_LengthError std::length_error
		\value Std_OutOfRange std::out_of_range
		\value Std_FutureError std::future_error
		\value Std_RuntimeError std::runtime_error
		\value Std_RangeError std::range_error
		\value Std_OverflowError std::overflow_error
		\value Std_UnderflowError std::underflow_error
		\value Std_RegexError std::regex_error
		\value Std_SystemError std::system_error
		\value Std_IOFailure std::ios_base::failure
		\value Std_FileSystemError std::filesystem::filesystem_error
		\value Std_TXException ISO/IEC TS 19841:2015 标准的事务异常类型，这是个技术预览内容，
			目前尚未被纳入C++标准，也没有关于它的从std::exception转换到此类型的支持。
		\value Std_LocalTimeNone std::chrono::nonexistent_local_time
		\value Std_LocalTimeAmbiguous std::chrono::ambiguous_local_time
		\value Std_FormatError std::format_error
		\value Std_BadTypeid std::bad_typeid
		\value Std_BadCast std::bad_cast
		\value Std_BadAnyCast std::bad_any_cast
		\value Std_BadOptionalAccess std::bad_optional_access
		\value Std_BadExpectedAccess std::bad_expected_access
		\value Std_BadWeakPtr std::bad_weak_ptr
		\value Std_BadFunctionCall std::bad_function_call
		\value Std_BadAlloc std::bad_alloc
		\value Std_BadArrayNewLength std::bad_array_new_length
		\value Std_BadException std::bad_exception
		\value Std_BadVariantAccess std::bad_variant_access
	*/
	/*!
		\since Visindigo 0.13.0
		构造函数，使用指定的参数创建一个异常对象。

		\a type 异常类型。
		\a message 异常消息本体。
		\a critical 是否为严重异常。
		\a file 发生异常的源文件名。
		\a line 发生异常的行号。
		\a func 函数名称。
	*/
	Exception::Exception(Type type, QString message, bool critical, QString file, int line, QString func,
	QList<StacktraceFrame> stacktrace) {
		d = new ExceptionPrivate();
		d->Type = type;
		d->Message = message;
		d->Critical = critical;
		d->File = file;
		d->Line = line;
		d->Function = func;
		d->Stacktrace = stacktrace;
	}

	/*!
		\fn Visindigo::General::Exception::Exception(const Exception& other)
		\since Visindigo 0.13.0

		拷贝构造函数
	*/

	/*!
		\fn Visindigo::General::Exception::Exception(Exception&& other) noexcept
		\since Visindigo 0.13.0

		移动构造函数
	*/

	/*!
		\fn Visindigo::General::Exception& Visindigo::General::Exception::operator=(const Exception& other)
		\since Visindigo 0.13.0

		拷贝赋值运算符
	*/

	/*!
		\fn Visindigo::General::Exception& Visindigo::General::Exception::operator=(Exception&& other) noexcept
		\since Visindigo 0.13.0

		移动赋值运算符
	*/
	VIMoveable_Impl(Exception);
	VICopyable_Impl(Exception);

	/*!
		\since Visindigo 0.13.0
		析构函数，销毁异常对象。
	*/
	Exception::~Exception() {
		if (d != nullptr) {
			delete d;
		}
	}

	/*!
		\since Visindigo 0.13.0
		return 异常消息本体

		要获取全部异常信息，请使用toString()函数。

		what()函数返回与此相同的内容。

		\sa what() toString()
	*/
	QString Exception::getMessage() const {
		return d->Message;
	}

	/*!
		\since Visindigo 0.13.0
		return 异常类型
	*/
	Exception::Type Exception::getType() const {
		return d->Type;
	}
	
	/*!
		\since Visindigo 0.13.0
		return 异常是否为严重异常

		如果程序理应立即终止，则异常应为严重异常。

		推荐将所有异常都设置为严重异常，除非您非常确定异常不会影响程序的正常运行。

		由于C++的灵活性，完整的异常安全性几乎无法保证，因此建议将大部分异常都视为严重异常，以确保程序的稳定性。
	*/
	bool Exception::isCritical() const {
		return d->Critical;
	}

	/*!
		\since Visindigo 0.13.0
		return 发生异常的源文件名
	*/
	QString Exception::getFile() const {
		return d->File;
	}

	/*!
		\since Visindigo 0.13.0
		return 发生异常的行号
	*/
	int Exception::getLine() const {
		return d->Line;
	}

	/*!
		\since Visindigo 0.13.0
		return 发生异常的函数名称
	*/
	QString Exception::getFunction() const {
		return d->Function;
	}

	/*!
		\since Visindigo 0.13.0
		return 异常的堆栈跟踪信息
	*/
	QList<StacktraceFrame> Exception::getStacktrace() const {
		return d->Stacktrace;
	}

	/*!
		\since Visindigo 0.13.0
		return 异常的标准异常描述字符串。

		此函数返回与getMessage()函数相同的内容。

		\sa getMessage()
	*/
	const char* Exception::what() const noexcept {
		return d->Message.toStdString().c_str();
	}

	/*!
		\since Visindigo 0.13.0
		return 将异常信息格式化为字符串表示形式。

		这是个用于调试的辅助函数，Visindigo输出日志时不依赖此函数，因此调用此函数
		输出的结果和在日志上看到的结果有所不同。
	*/
	QString Exception::toString() const {
		QString typeStr = QString::number(static_cast<int>(d->Type));
		QString criticalStr = d->Critical ? "Yes" : "No";
		QString stacktraceStr = "";
		for (const StacktraceFrame& frame : d->Stacktrace) {
			stacktraceStr += "\n" + frame.toString();
		}
		return QString("Exception Type: %1\nMessage: %2\nCritical: %3\nFile: %4\nLine: %5\nFunction: %6\nStacktrace:%7")
			.arg(typeStr)
			.arg(d->Message)
			.arg(criticalStr)
			.arg(d->File)
			.arg(d->Line)
			.arg(d->Function)
			.arg(stacktraceStr);
	}
}