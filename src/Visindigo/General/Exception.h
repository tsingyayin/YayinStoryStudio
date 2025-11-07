#ifndef Visindigo_General_Exception_H
#define Visindigo_General_Exception_H
#include "../Macro.h"
#include <QtCore/qstring.h>
namespace Visindigo::General {
	class ExceptionPrivate;
	class VisindigoAPI Exception {
	public:
		enum Type {
			Unknown,
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
			Other
		};
	public:
		static Exception fromStdException(const std::exception& e);
	public:
		Exception(Type type, QString message = "", bool critical = true, QString file = "", int line = 0, QString func = "");
		Exception(const Exception& other);
		Exception& operator=(const Exception& other);
		Exception(Exception&& other) noexcept;
		Exception& operator=(Exception&& other) noexcept;
		virtual ~Exception();
		QString getMessage() const;
		Type getType() const;
		bool isCritical() const;
		QString getFile() const;
		int getLine() const;
		QString getFunction() const;
		const char* what() const noexcept;
	private:
		ExceptionPrivate* d;
	};
}
#endif // Visindigo_General_Exception_H

#define VI_Throw(type, message) throw Visindigo::General::Exception(type, message, true, __FILE__, __LINE__, Q_FUNC_INFO)
