#ifndef YSSCore_General_YSSFileError_h
#define YSSCore_General_YSSFileError_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>
// Forward declarations
namespace YSSCore::General {
	class YSSFileErrorPrivate;
}
// Main
namespace YSSCore::General {
	class YSSCoreAPI YSSFileError {
		Q_GADGET;
	public:
		enum class ErrorType {
			NoError = 0,
			NotFound,
			PermissionDenied,
			AlreadyExists,
			NotADirectory,
			IoError,
			NotSupported,
			Aborted,
			InvalidConfiguration,
			FileTooLarge,
			NeedsCredentials,
			Unknown
		};
		Q_ENUM(ErrorType)
	public:
		YSSFileError();
		VICopyable(YSSFileError);
		VIMoveable(YSSFileError);
		~YSSFileError();
	public:
		static QString errorTypeName(ErrorType type);
	public:
		ErrorType getType() const;
		QString getMessage() const;
		QString getDetail() const;
	public:
		void setType(ErrorType type);
		void setMessage(const QString& message);
		void setDetail(const QString& detail);
	public:
		bool isError() const;
		QString toString() const;
	private:
		YSSFileErrorPrivate* d;
	};
}
#endif // YSSCore_General_YSSFileError_h
