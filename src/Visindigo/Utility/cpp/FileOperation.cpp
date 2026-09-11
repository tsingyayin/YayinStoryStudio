#include <expected>
#include <QtCore/qbytearray.h>
#include <QtCore/qdir.h>
#include <QtCore/qdirlisting.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtextstream.h>
#include "Utility/FileOperation.h"

namespace Visindigo::Utility {
	class FileOperationPrivate {
		friend class FileOperation;
	protected:
		// 这个函数只应该在操作失败时调用，它把Qt的底层错误转换为ErrorCode。
		// 由于Qt并不提供磁盘空间不足一类的精细错误码，无法进一步判定的失败会返回UnknownError。
		static FileOperation::ErrorCode mapFileError(QFileDevice::FileError error) {
			switch (error) {
			case QFileDevice::PermissionsError:
				return FileOperation::PermissionDenied;
			case QFileDevice::WriteError:
			case QFileDevice::FatalError:
				// 在文件已经成功打开的前提下，写入失败最常见的原因是磁盘空间不足
				return FileOperation::DiskFull;
			default:
				return FileOperation::UnknownError;
			}
		}
	};

	/*!
		\class Visindigo::Utility::FileOperation
		\brief 此类为Yayin Story Studio 提供带有错误码的文件操作的相关函数.
		\since Visindigo 0.17.0
		\inmodule Visindigo

		这类所有函数都是静态函数，所以你不需要创建它的实例。

		与 Visindigo::Utility::FileUtility 不同，这里的所有函数都不会自行吞掉错误：
		读取类的函数返回 Errorable（即 std::expected），写入和删除类的函数返回 ErrorCode，
		调用方可以据此决定是记录日志、重试还是回退。FileUtility 中对应的旧函数自0.17.0起已废弃，
		它们只是转发到这里并保持旧有的容错行为。
	*/

	/*!
		\enum Visindigo::Utility::FileOperation::ErrorCode
		此枚举用于表示文件操作的结果
		\since Visindigo 0.17.0
		\value Success 操作成功
		\value FileNotFound 指定的文件不存在
		\value DirNotFound 指定的目录不存在，或者所需的上级目录无法创建
		\value NameConflict 目标已存在，且调用方没有要求覆盖它
		\value PermissionDenied 没有访问该文件或目录的权限
		\value DiskFull 写入失败，通常是磁盘空间不足导致的
		\value UnknownError 无法进一步判定的错误
	*/

	/*!
		\since Visindigo 0.17.0
		\a code 错误码

		return 错误码的名字，主要用于日志输出。
	*/
	QString FileOperation::errorCodeName(ErrorCode code) {
		switch (code) {
		case Success:
			return QStringLiteral("Success");
		case FileNotFound:
			return QStringLiteral("FileNotFound");
		case DirNotFound:
			return QStringLiteral("DirNotFound");
		case NameConflict:
			return QStringLiteral("NameConflict");
		case PermissionDenied:
			return QStringLiteral("PermissionDenied");
		case DiskFull:
			return QStringLiteral("DiskFull");
		default:
			return QStringLiteral("UnknownError");
		}
	}

	/*!
		\since Visindigo 0.17.0
		\a filePath 文件路径

		return 以行列表的形式读取文件的内容，如果文件不存在则返回FileNotFound，
		如果文件无法打开或者在读取过程中出错，则返回对应的ErrorCode。
	*/
	FileOperation::Errorable<QStringList> FileOperation::readLines(const QString& filePath) {
		QFile file(filePath);
		if (!file.exists()) {
			return std::unexpected(FileNotFound);
		}
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			return std::unexpected(FileOperationPrivate::mapFileError(file.error()));
		}
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		QStringList rtn;
		while (!ts.atEnd()) {
			rtn.append(ts.readLine());
		}
		const QFileDevice::FileError error = file.error();
		const QTextStream::Status status = ts.status();
		file.close();
		if (status != QTextStream::Ok || error != QFileDevice::NoError) {
			return std::unexpected(FileOperationPrivate::mapFileError(error));
		}
		return rtn;
	}

	/*!
		\since Visindigo 0.17.0
		\a filePath 文件路径

		return 文件的全部文本内容，如果文件不存在则返回FileNotFound，
		如果文件无法打开或者在读取过程中出错，则返回对应的ErrorCode。
	*/
	FileOperation::Errorable<QString> FileOperation::readAll(const QString& filePath) {
		QFile file(filePath);
		if (!file.exists()) {
			return std::unexpected(FileNotFound);
		}
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			return std::unexpected(FileOperationPrivate::mapFileError(file.error()));
		}
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		const QString rtn = ts.readAll();
		const QFileDevice::FileError error = file.error();
		const QTextStream::Status status = ts.status();
		file.close();
		if (status != QTextStream::Ok || error != QFileDevice::NoError) {
			return std::unexpected(FileOperationPrivate::mapFileError(error));
		}
		return rtn;
	}

	/*!
		\since Visindigo 0.17.0
		\a filePath 文件路径

		return 文件的全部二进制内容，如果文件不存在则返回FileNotFound，
		如果文件无法打开或者在读取过程中出错，则返回对应的ErrorCode。

		\note 与 readAll 不同，这个函数不进行任何编码和换行符转换。
	*/
	FileOperation::Errorable<QByteArray> FileOperation::readBinary(const QString& filePath) {
		QFile file(filePath);
		if (!file.exists()) {
			return std::unexpected(FileNotFound);
		}
		if (!file.open(QIODevice::ReadOnly)) {
			return std::unexpected(FileOperationPrivate::mapFileError(file.error()));
		}
		const QByteArray rtn = file.readAll();
		const QFileDevice::FileError error = file.error();
		file.close();
		if (error != QFileDevice::NoError) {
			return std::unexpected(FileOperationPrivate::mapFileError(error));
		}
		return rtn;
	}

	/*!
		\since Visindigo 0.17.0
		\a filePath 文件路径
		\a lines 需要保存的行列表
		\a joinLine 行连接符

		将QStringList保存到文件中，行与行之间用joinLine连接。
		如果目标文件不存在，则连同其上级目录一起创建。

		return 操作结果。如果上级目录无法创建则返回DirNotFound，
		如果文件无法写入则返回对应的ErrorCode。
	*/
	FileOperation::ErrorCode FileOperation::saveLines(const QString& filePath, const QStringList& lines, const QString& joinLine) {
		return saveAll(filePath, lines.join(joinLine));
	}

	/*!
		\since Visindigo 0.17.0
		\a filePath 文件路径
		\a data 需要保存的文本数据

		将QString保存到文件中。如果目标文件不存在，则连同其上级目录一起创建。

		return 操作结果。如果上级目录无法创建则返回DirNotFound，
		如果文件无法写入则返回对应的ErrorCode。

		\note 这个函数无法感知对象的析构，因此保存是即时完成的，不会进行延迟写入。
	*/
	FileOperation::ErrorCode FileOperation::saveAll(const QString& filePath, const QString& data) {
		QFile file(filePath);
		const bool exist = file.exists();
		const QDir parentDir = QFileInfo(file).absoluteDir();
		if (!parentDir.exists() && !parentDir.mkpath(".")) {
			return DirNotFound;
		}
		if (!file.open((exist ? QIODevice::WriteOnly : QIODevice::NewOnly) | QIODevice::Text)) {
			return FileOperationPrivate::mapFileError(file.error());
		}
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		ts << data;
		ts.flush();
		QFileDevice::FileError error = file.error();
		if (error == QFileDevice::NoError && ts.status() != QTextStream::Ok) {
			error = QFileDevice::WriteError;
		}
		file.close();
		if (error != QFileDevice::NoError) {
			return FileOperationPrivate::mapFileError(error);
		}
		return Success;
	}

	/*!
		\since Visindigo 0.17.0
		\a filePath 文件路径
		\a data 需要保存的二进制数据

		将QByteArray保存到文件中。如果目标文件不存在，则连同其上级目录一起创建。

		return 操作结果。如果上级目录无法创建则返回DirNotFound，
		如果文件无法写入则返回对应的ErrorCode。

		\note 与 saveAll 不同，这个函数不进行任何编码和换行符转换。
	*/
	FileOperation::ErrorCode FileOperation::saveBinary(const QString& filePath, const QByteArray& data) {
		QFile file(filePath);
		const bool exist = file.exists();
		const QDir parentDir = QFileInfo(file).absoluteDir();
		if (!parentDir.exists() && !parentDir.mkpath(".")) {
			return DirNotFound;
		}
		if (!file.open(exist ? QIODevice::WriteOnly : QIODevice::NewOnly)) {
			return FileOperationPrivate::mapFileError(file.error());
		}
		const qint64 written = file.write(data);
		QFileDevice::FileError error = file.error();
		if (error == QFileDevice::NoError && written != data.size()) {
			error = QFileDevice::WriteError;
		}
		file.close();
		if (error != QFileDevice::NoError) {
			return FileOperationPrivate::mapFileError(error);
		}
		return Success;
	}

	/*!
		\since Visindigo 0.17.0
		\a filePath 文件路径
		\a moveToTrash 是否移动到回收站

		删除指定文件。如果moveToTrash为true，则把文件移动到回收站，否则直接删除。

		return 操作结果。如果文件不存在则返回FileNotFound，
		如果删除失败则返回对应的ErrorCode。
	*/
	FileOperation::ErrorCode FileOperation::deleteFile(const QString& filePath, bool moveToTrash) {
		QFile file(filePath);
		if (!file.exists()) {
			return FileNotFound;
		}
		if (moveToTrash) {
			if (file.moveToTrash()) {
				return Success;
			}
			return FileOperationPrivate::mapFileError(file.error());
		}
		if (file.remove()) {
			return Success;
		}
		return FileOperationPrivate::mapFileError(file.error());
	}

	/*!
		\since Visindigo 0.17.0
		\a srcPath 源文件路径
		\a dstPath 目标文件路径
		\a rinse 是否使用漂洗的方式复制文件
		\a overwrite 是否覆盖已存在的目标文件

		复制文件。如果rinse为true，则使用漂洗的方式复制文件，否则使用QFile::copy()，
		如果overwrite为true，则覆盖已存在的目标文件，否则不进行复制。

		漂洗模式只忠实传递文件本体的二进制数据，不从源文件读取任何元数据，也不写入任何元数据到目标文件，
		因此在某些特殊情况下可能会得到一个与源文件不同的目标文件，例如当源文件具有特殊权限时，目标文件可能会得到默认权限；
		当源文件具有特殊属性时，目标文件可能不会继承这些属性；当源文件具有特殊时间戳时，目标文件可能会得到当前时间戳。

		这对于从qrc编译到二进制文件内的资源文件向外部复制时很有用，因为qrc资源文件会自动设为只读，
		并且具有特殊的权限和属性，使用漂洗模式复制可以得到一个正常的可读写文件。

		return 操作结果。如果源文件不存在则返回FileNotFound，如果目标已存在且overwrite为false则返回NameConflict，
		其余失败返回对应的ErrorCode。
	*/
	FileOperation::ErrorCode FileOperation::copyFile(const QString& srcPath, const QString& dstPath, bool rinse, bool overwrite) {
		QFile srcFile(srcPath);
		if (!srcFile.exists()) {
			return FileNotFound;
		}
		QFile dstFile(dstPath);
		const QFileInfo dstFileInfo(dstFile);
		if (dstFile.exists()) {
			if (not overwrite) {
				return NameConflict;
			}
			if (not dstFile.remove()) {
				return FileOperationPrivate::mapFileError(dstFile.error());
			}
		}
		const QDir dstDir = dstFileInfo.absoluteDir();
		if (not dstDir.exists() && not dstDir.mkpath(".")) {
			return DirNotFound;
		}
		if (not rinse) {
			if (srcFile.copy(dstPath)) {
				return Success;
			}
			return FileOperationPrivate::mapFileError(srcFile.error());
		}
		if (not srcFile.open(QIODevice::ReadOnly)) {
			return FileOperationPrivate::mapFileError(srcFile.error());
		}
		const QByteArray data = srcFile.readAll();
		const QFileDevice::FileError readError = srcFile.error();
		srcFile.close();
		if (readError != QFileDevice::NoError) {
			return FileOperationPrivate::mapFileError(readError);
		}
		if (not dstFile.open(QIODevice::WriteOnly)) {
			return FileOperationPrivate::mapFileError(dstFile.error());
		}
		const qint64 written = dstFile.write(data);
		QFileDevice::FileError writeError = dstFile.error();
		if (writeError == QFileDevice::NoError && written != data.size()) {
			writeError = QFileDevice::WriteError;
		}
		dstFile.close();
		if (writeError != QFileDevice::NoError) {
			return FileOperationPrivate::mapFileError(writeError);
		}
		return Success;
	}

	/*!
		\since Visindigo 0.17.0
		\a srcPath 源文件路径
		\a dstPath 目标文件路径
		\a rinse 是否使用漂洗的方式移动文件
		\a overwrite 是否覆盖已存在的目标文件

		移动文件。当rinse为false时，使用QFile::rename()直接重命名移动；当rinse为true时，
		先漂洗复制再删除源文件，适用于跨卷移动或需要剥离元数据的场景。

		return 操作结果。如果源文件不存在则返回FileNotFound，如果目标已存在且overwrite为false则返回NameConflict，
		如果目标路径的上级目录不存在则返回DirNotFound，其余失败返回对应的ErrorCode。
	*/
	FileOperation::ErrorCode FileOperation::moveFile(const QString& srcPath, const QString& dstPath, bool rinse, bool overwrite) {
		QFile srcFile(srcPath);
		if (!srcFile.exists()) {
			return FileNotFound;
		}
		QFile dstFile(dstPath);
		if (dstFile.exists()) {
			if (not overwrite) {
				return NameConflict;
			}
			if (not dstFile.remove()) {
				return FileOperationPrivate::mapFileError(dstFile.error());
			}
		}
		if (not QFileInfo(dstPath).absoluteDir().exists()) {
			return DirNotFound;
		}
		if (not rinse) {
			if (srcFile.rename(dstPath)) {
				return Success;
			}
			return FileOperationPrivate::mapFileError(srcFile.error());
		}
		if (not srcFile.open(QIODevice::ReadOnly)) {
			return FileOperationPrivate::mapFileError(srcFile.error());
		}
		const QByteArray data = srcFile.readAll();
		const QFileDevice::FileError readError = srcFile.error();
		srcFile.close();
		if (readError != QFileDevice::NoError) {
			return FileOperationPrivate::mapFileError(readError);
		}
		if (not dstFile.open(QIODevice::WriteOnly)) {
			return FileOperationPrivate::mapFileError(dstFile.error());
		}
		const qint64 written = dstFile.write(data);
		QFileDevice::FileError writeError = dstFile.error();
		if (writeError == QFileDevice::NoError && written != data.size()) {
			writeError = QFileDevice::WriteError;
		}
		dstFile.close();
		if (writeError != QFileDevice::NoError) {
			return FileOperationPrivate::mapFileError(writeError);
		}
		if (not srcFile.remove()) {
			return FileOperationPrivate::mapFileError(srcFile.error());
		}
		return Success;
	}

	/*!
		\since Visindigo 0.17.0
		\a dirPath 目录路径
		\a moveToTrash 是否移动到回收站

		删除指定目录及其所有内容。如果moveToTrash为true，则把整个目录移动到回收站，否则直接递归删除。

		return 操作结果。如果目录不存在则返回DirNotFound，其余失败返回对应的ErrorCode。

		\warning 当moveToTrash为true时，是否支持把整个目录移动到回收站取决于平台，
		在不支持的平台上会返回UnknownError，此时调用方需要自行决定是否改用直接删除。
		\note 直接删除时会递归删除目录中的所有文件和子目录，请谨慎使用。
		\note 此函数没有 Visindigo::Utility::FileUtility::deleteDir 的exclude参数，
		需要保留部分内容的调用方请继续使用 FileUtility 中的版本。
	*/
	FileOperation::ErrorCode FileOperation::deleteDir(const QString& dirPath, bool moveToTrash) {
		QDir dir(dirPath);
		if (!dir.exists()) {
			return DirNotFound;
		}
		if (moveToTrash) {
			if (not QFile::supportsMoveToTrash()) {
				return UnknownError;
			}
			if (QFile::moveToTrash(dirPath)) {
				return Success;
			}
			return UnknownError;
		}
		if (dir.removeRecursively()) {
			return Success;
		}
		return UnknownError;
	}

	/*!
		\since Visindigo 0.17.0
		\a srcPath 源目录路径
		\a dstPath 目标目录路径
		\a rinse 是否使用漂洗的方式复制文件
		\a overwrite 是否覆盖已存在的目标文件

		复制整个目录。此函数会遍历源目录中的所有文件，并对每个文件调用copyFile()。

		return 操作结果。如果源目录不存在则返回DirNotFound，否则返回第一个出错文件的错误码，
		但在出错后仍会继续尝试复制剩余的文件，因此这个函数可能造成部分复制的结果。
	*/
	FileOperation::ErrorCode FileOperation::copyDir(const QString& srcPath, const QString& dstPath, bool rinse, bool overwrite) {
		QDir srcDir(srcPath);
		if (!srcDir.exists()) {
			return DirNotFound;
		}
		QDirListing list(srcPath, QStringList() << "*", QDirListing::IteratorFlag::Recursive | QDirListing::IteratorFlag::FilesOnly);
		ErrorCode firstError = Success;
		for (const auto& entry : list) {
			const QString srcFilePath = entry.absoluteFilePath();
			const QString dstFilePath = QDir(dstPath).absoluteFilePath(srcDir.relativeFilePath(srcFilePath));
			const QDir dstFileDir = QFileInfo(dstFilePath).absoluteDir();
			if (not dstFileDir.exists() && not dstFileDir.mkpath(".")) {
				if (firstError == Success) {
					firstError = DirNotFound;
				}
				continue;
			}
			const ErrorCode error = copyFile(srcFilePath, dstFilePath, rinse, overwrite);
			if (error != Success && firstError == Success) {
				firstError = error;
			}
		}
		return firstError;
	}

	/*!
		\since Visindigo 0.17.0
		\a srcPath 源目录路径
		\a dstPath 目标目录路径
		\a rinse 是否使用漂洗的方式移动文件
		\a overwrite 是否覆盖已存在的目标文件

		移动整个目录。此函数会遍历源目录中的所有文件，并对每个文件调用moveFile()。
		只有当所有文件都移动成功时，才会删除此时已经为空的源目录结构。

		return 操作结果。如果源目录不存在则返回DirNotFound，否则返回第一个出错文件的错误码，
		但在出错后仍会继续尝试移动剩余的文件，因此这个函数可能造成部分移动的结果。
	*/
	FileOperation::ErrorCode FileOperation::moveDir(const QString& srcPath, const QString& dstPath, bool rinse, bool overwrite) {
		QDir srcDir(srcPath);
		if (!srcDir.exists()) {
			return DirNotFound;
		}
		QDirListing list(srcPath, QStringList() << "*", QDirListing::IteratorFlag::Recursive | QDirListing::IteratorFlag::FilesOnly);
		ErrorCode firstError = Success;
		for (const auto& entry : list) {
			const QString srcFilePath = entry.absoluteFilePath();
			const QString dstFilePath = QDir(dstPath).absoluteFilePath(srcDir.relativeFilePath(srcFilePath));
			const QDir dstFileDir = QFileInfo(dstFilePath).absoluteDir();
			if (not dstFileDir.exists() && not dstFileDir.mkpath(".")) {
				if (firstError == Success) {
					firstError = DirNotFound;
				}
				continue;
			}
			const ErrorCode error = moveFile(srcFilePath, dstFilePath, rinse, overwrite);
			if (error != Success && firstError == Success) {
				firstError = error;
			}
		}
		if (firstError == Success && not srcDir.removeRecursively()) {
			// 源目录中可能残留了未被移动的内容，此时不做处理，但仍然报告失败
			firstError = UnknownError;
		}
		return firstError;
	}
}
