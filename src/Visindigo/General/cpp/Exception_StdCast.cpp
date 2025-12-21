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