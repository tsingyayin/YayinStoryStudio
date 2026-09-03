#include <any>
#include <chrono>
#include <expected>
#include <filesystem>
#include <future>
#include <regex>
#include <stdexcept>
#include <QtCore/qmetaobject.h>
#include "General/Exception.h"
#define isTypeOf(exceptionType) dynamic_cast<const exceptionType*>(&e) != nullptr
// C++20 <chrono> 的 nonexistent_local_time / ambiguous_local_time 依赖标准库的时区数据库(tzdb)。
// NDK/libc++ 默认未启用该功能，引用这两个类型会直接编译失败；MSVC STL / libstdc++ 均已提供。
// 因此在 libc++ 且无 tzdb 时屏蔽这两条转换（不影响其它平台/标准库）。
#if defined(_LIBCPP_VERSION) && \
	(!defined(_LIBCPP_HAS_TIME_ZONE_DATABASE) || !_LIBCPP_HAS_TIME_ZONE_DATABASE)
#define VI_HAS_NO_CHRONO_LOCAL_TIME_EXCEPTIONS 1
#endif
// NDK 26.x 的 libc++：<format> 支持未启用，std::format_error 的 RTTI typeinfo 未随
// libc++.so 导出，dynamic_cast 引用其 typeid 会在链接期失败；MSVC STL / libstdc++ 无此问题。
#if defined(_LIBCPP_VERSION) && defined(__ANDROID__)
#define VI_HAS_NO_STD_FORMAT_ERROR 1
#endif
namespace Visindigo::General {
	/*!
		\since Visindigo 0.13.0
		\a type 错误类型枚举值

		return 将异常类型枚举值转换为字符串表示形式。
	*/
	QString Exception::typeToString(Type type) {
		return QMetaEnum::fromType<Type>().valueToKey(static_cast<int>(type));
	}
	/*!
		\since Visindigo 0.13.0
		\a e 标准异常对象

		return 标准异常对象转换为 Visindigo::General::Exception 对象。

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
#ifndef VI_HAS_NO_CHRONO_LOCAL_TIME_EXCEPTIONS
			else if (isTypeOf(std::chrono::nonexistent_local_time)) { type = Std_LocalTimeNone; }
			else if (isTypeOf(std::chrono::ambiguous_local_time)) { type = Std_LocalTimeAmbiguous; }
#endif
#ifndef VI_HAS_NO_STD_FORMAT_ERROR
			else if (isTypeOf(std::format_error)) { type = Std_FormatError; }
#endif
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
		}
		catch (...) {
			type = Type::Std_Unknown;
		}
		return Exception(type, message);
	}
}