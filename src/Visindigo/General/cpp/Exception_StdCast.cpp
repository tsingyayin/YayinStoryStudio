#include "General/Exception.h"
#include <stdexcept>
#include <future>
#include <regex>
#include <any>
#include <expected>
#include <chrono>
#include <filesystem>
#define isTypeOf(exceptionType) dynamic_cast<const exceptionType*>(&e) != nullptr
namespace Visindigo::General {
	/*!
		\since Visindigo 0.13.0
		将异常类型枚举值转换为字符串表示形式。
	*/
	QString Exception::typeToString(Type type) {
		switch (type) {
		case Unknown: return "Unknown";
		case Other: return "Other";
		case IOError: return "IOError";
		case ParseError: return "ParseError";
		case InvalidArgument: return "InvalidArgument";
		case OutOfRange: return "OutOfRange";
		case NotFound: return "NotFound";
		case AlreadyExists: return "AlreadyExists";
		case PermissionDenied: return "PermissionDenied";
		case Timeout: return "Timeout";
		case UnsupportedOperation: return "UnsupportedOperation";
		case InternalError: return "InternalError";
		case OutOfMemory: return "OutOfMemory";
		case Std_Unknown: return "Std_Unknown";
		case Std_Other: return "Std_Other";
		case Std_LogicError: return "Std_LogicError";
		case Std_InvalidArgument: return "Std_InvalidArgument";
		case Std_DomainError: return "Std_DomainError";
		case Std_LengthError: return "Std_LengthError";
		case Std_OutOfRange: return "Std_OutOfRange";
		case Std_FutureError: return "Std_FutureError";
		case Std_RuntimeError: return "Std_RuntimeError";
		case Std_RangeError: return "Std_RangeError";
		case Std_OverflowError: return "Std_OverflowError";
		case Std_UnderflowError: return "Std_UnderflowError";
		case Std_RegexError: return "Std_RegexError";
		case Std_SystemError: return "Std_SystemError";
		case Std_IOFailure: return "Std_IOFailure";
		case Std_FileSystemError: return "Std_FileSystemError";
		case Std_TXException: return "Std_TXException";
		case Std_LocalTimeNone: return "Std_LocalTimeNone";
		case Std_LocalTimeAmbiguous: return "Std_LocalTimeAmbiguous";
		case Std_FormatError: return "Std_FormatError";
		case Std_BadTypeid: return "Std_BadTypeid";
		case Std_BadCast: return "Std_BadCast";
		case Std_BadAnyCast: return "Std_BadAnyCast";
		case Std_BadOptionalAccess: return "Std_BadOptionalAccess";
		case Std_BadExpectedAccess: return "Std_BadExpectedAccess";
		case Std_BadWeakPtr: return "Std_BadWeakPtr";
		case Std_BadFunctionCall: return "Std_BadFunctionCall";
		case Std_BadAlloc: return "Std_BadAlloc";
		case Std_BadArrayNewLength: return "Std_BadArrayNewLength";
		case Std_BadException: return "Std_BadException";
		case Std_BadVariantAccess: return "Std_BadVariantAccess";
			default: return "Unknown";
		}
	}
	/*!
		\since Visindigo 0.13.0
		\a e 标准异常对象
		将标准异常对象转换为 Visindigo::General::Exception 对象。

		由于标准异常对象的信息有限，转换后的异常对象的类型将被设置为 Unknown，
		消息将包含标准异常的 what() 信息，其他信息（如文件名、行号、函数名和堆栈跟踪）将为空或默认值。
	*/
	Exception Exception::fromStdException(const std::exception& e) {
		Type type = Type::Std_Unknown;
		QString message = QString::fromUtf8(e.what());
		try {
			if (false) {} // this line use to start if chain
			else if (isTypeOf(std::future_error)) { type = Std_FutureError; }
			else if (isTypeOf(std::out_of_range)) { type = Std_OutOfRange; }
			else if (isTypeOf(std::length_error)) { type = Std_LengthError; }
			else if (isTypeOf(std::domain_error)) { type = Std_DomainError; }
			else if (isTypeOf(std::invalid_argument)) { type = Std_InvalidArgument; }
			else if (isTypeOf(std::logic_error)) { type = Std_LogicError; }

			else if (isTypeOf(std::range_error)) { type = Std_RangeError; }
			else if (isTypeOf(std::overflow_error)) { type = Std_OverflowError; }
			else if (isTypeOf(std::underflow_error)) { type = Std_UnderflowError; }
			else if (isTypeOf(std::regex_error)) { type = Std_RegexError; }
			else if (isTypeOf(std::system_error)) { type = Std_SystemError; }
			else if (isTypeOf(std::ios_base::failure)) { type = Std_IOFailure; }
			else if (isTypeOf(std::filesystem::filesystem_error)) { type = Std_FileSystemError; }
			else if (isTypeOf(std::chrono::nonexistent_local_time)) { type = Std_LocalTimeNone; }
			else if (isTypeOf(std::chrono::ambiguous_local_time)) { type = Std_LocalTimeAmbiguous; }
			else if (isTypeOf(std::format_error)) { type = Std_FormatError; }
			else if (isTypeOf(std::runtime_error)) { type = Std_RuntimeError; }

			else if (isTypeOf(std::bad_typeid)) { type = Std_BadTypeid; }
			else if (isTypeOf(std::bad_cast)) { type = Std_BadCast; }
			else if (isTypeOf(std::bad_any_cast)) { type = Std_BadAnyCast; }
			else if (isTypeOf(std::bad_optional_access)) { type = Std_BadOptionalAccess; }
			else if (isTypeOf(std::bad_expected_access<void>)) { type = Std_BadExpectedAccess; }
			else if (isTypeOf(std::bad_weak_ptr)) { type = Std_BadWeakPtr; }
			else if (isTypeOf(std::bad_function_call)) { type = Std_BadFunctionCall; }
			else if (isTypeOf(std::bad_alloc)) { type = Std_BadAlloc; }
			else if (isTypeOf(std::bad_array_new_length)) { type = Std_BadArrayNewLength; }
			else if (isTypeOf(std::bad_exception)) { type = Std_BadException; }
			else if (isTypeOf(std::bad_variant_access)) { type = Std_BadVariantAccess; }
			else {
				type = Type::Std_Other;
			}
		} catch (...) {
			type = Type::Std_Unknown;
		}
		return Exception(type, message);
	}
}