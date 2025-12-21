#ifndef Visindigo_General_Exception_H
#define Visindigo_General_Exception_H
#include "../Macro.h"
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include "General/StacktraceHelper.h"
namespace Visindigo::General {
	class StacktraceFrame;
	class ExceptionPrivate;
	class VisindigoAPI Exception {
	public:
		enum Type : qint32{
			Unknown = 0x0000000,
			Other,

			IOError,
			ParseError,
			InvalidArgument,
			OutOfRange,
			NotFound,
			AlreadyExists,
			PermissionDenied,
			Timeout,
			UnsupportedOperation,
			InternalError,
			OutOfMemory,
			
			// Following types are aligned with std::exception derived types
			Std_Unknown = 0x8000000,
			Std_Other,
			Std_LogicError,
				Std_InvalidArgument,
				Std_DomainError,
				Std_LengthError,
				Std_OutOfRange,
				Std_FutureError, // since C++ 11
			Std_RuntimeError,
				Std_RangeError,
				Std_OverflowError,
				Std_UnderflowError,
				Std_RegexError, // since C++ 11
				Std_SystemError, // since C++ 11
					Std_IOFailure, // since C++ 11, std::ios_base::failure
					Std_FileSystemError, // since C++ 17, std::filesystem::filesystem_error
				Std_TXException, // ISO/IEC TS 19841:2015, not in C++ standard yet
				Std_LocalTimeNone, // since C++ 20, std::chrono::nonexistent_local_time
				Std_LocalTimeAmbiguous, // since C++ 20, std::chrono::ambiguous_local_time
				Std_FormatError, // since C++ 20
			Std_BadTypeid,
			Std_BadCast,
				Std_BadAnyCast, // since C++ 17
			Std_BadOptionalAccess, // since C++ 17
			Std_BadExpectedAccess, // since C++ 23
			Std_BadWeakPtr, // since C++ 11
			Std_BadFunctionCall, // since C++ 11
			Std_BadAlloc,
				Std_BadArrayNewLength, // since C++ 11
			Std_BadException,
			Std_BadVariantAccess, // since C++ 17
		};
	public:
		static Exception fromStdException(const std::exception& e);
	public:
		Exception(Type type, QString message = "", 
			bool critical = true, QString file = "", 
			int line = 0, QString func = "",
			QList<StacktraceFrame> stacktrace = {});
		VIMoveable(Exception);
		VICopyable(Exception);
		virtual ~Exception();
		QString getMessage() const;
		Type getType() const;
		bool isCritical() const;
		QString getFile() const;
		int getLine() const;
		QString getFunction() const;
		QList<StacktraceFrame> getStacktrace() const;
		const char* what() const noexcept;
		QString toString() const;
	private:
		ExceptionPrivate* d;
	};
}
#endif // Visindigo_General_Exception_H

#define VI_Throw(type, message) throw Visindigo::General::Exception(type, message, true, __FILE__, __LINE__, Q_FUNC_INFO)
#define VI_Throw_ST(type, message) throw Visindigo::General::Exception(type, message, true, __FILE__, __LINE__, Q_FUNC_INFO, Visindigo::General::StacktraceHelper::getStacktrace())