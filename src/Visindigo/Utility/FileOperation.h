#ifndef Visindigo_Utility_FileOperation_h
#define Visindigo_Utility_FileOperation_h
#include "VICompileMacro.h"
#include <QtCore/qstring.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <expected>
namespace Visindigo::Utility {
	class VisindigoAPI FileOperation {
	public:
		enum ErrorCode {
			Success = 0,
			FileNotFound = 1,
			DirNotFound,
			NameConflict,
			PermissionDenied,
			DiskFull,
			UnknownError
		};
		template<typename T> using Errorable = std::expected<T, ErrorCode>;
	public:
		static QString errorCodeName(ErrorCode code);
		static Errorable<QStringList> readLines(const QString& filePath);
		static Errorable<QString> readAll(const QString& filePath);
		static Errorable<QByteArray> readBinary(const QString& filePath);
		static ErrorCode saveLines(const QString& filePath, const QStringList& lines, const QString& joinLine = "\n");
		static ErrorCode saveAll(const QString& filePath, const QString& data);
		static ErrorCode saveBinary(const QString& filePath, const QByteArray& data);
		static ErrorCode deleteFile(const QString& filePath, bool moveToTrash = true);
		static ErrorCode copyFile(const QString& srcPath, const QString& dstPath, bool rinse = true, bool overwrite = false);
		static ErrorCode moveFile(const QString& srcPath, const QString& dstPath, bool rinse = true, bool overwrite = false);
		static ErrorCode deleteDir(const QString& dirPath, bool moveToTrash = true);
		static ErrorCode copyDir(const QString& srcPath, const QString& dstPath, bool rinse = true, bool overwrite = false);
		static ErrorCode moveDir(const QString& srcPath, const QString& dstPath, bool rinse = true, bool overwrite = false);
	};
}
#endif // Visindigo_Utility_FileOperation_h