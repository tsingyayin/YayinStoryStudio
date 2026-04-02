#include "../FileUtility.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <QtCore/qtextstream.h>
#include <QtGui/qdesktopservices.h>
#include <QtCore/qurl.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qcoreapplication.h>
#include "General/Log.h"
#define VI_ENUMSTR(enumName, enumValue) case enumName::enumValue: return #enumValue;

namespace Visindigo::Utility {
	class FileUtilityPrivate {
		friend class FileUtility;
	protected:
		static QString getCountingUnitStr(FileUtility::CountingUnit c) {
			switch (c)
			{
			case FileUtility::_0:
				return "";
				break;
				VI_ENUMSTR(FileUtility, K);
				VI_ENUMSTR(FileUtility, M);
				VI_ENUMSTR(FileUtility, G);
				VI_ENUMSTR(FileUtility, T);
				VI_ENUMSTR(FileUtility, P);
				VI_ENUMSTR(FileUtility, E);
				VI_ENUMSTR(FileUtility, Z);
				VI_ENUMSTR(FileUtility, Y);
			default:
				return "";
			}
		}
		static QString getReadableSize(double rawSize, FileUtility::BinarySizeUnit u, FileUtility::CountingUnit c, FileUtility::BinarySizeFormat f) {
			if (rawSize >= (int)f) {
				return getReadableSize(rawSize / (int)f, u, (FileUtility::CountingUnit)(c + 1), f);
			}
			else {
				QString suffix = QString::number(rawSize, 'f', 2);
				suffix += getCountingUnitStr(c);
				if (f == FileUtility::IEC && c != FileUtility::_0) {
					suffix += "i";
				}
				if (u == FileUtility::Byte) {
					suffix += "B";
				}
				else {
					suffix += "b";
				}
				return suffix;
			}
		}
	};
	/*!
		\class Visindigo::Utility::FileUtility
		\brief 此类为Yayin Story Studio 提供文件操作的相关函数
		\since Yayin Story Studio 0.13.0
		\inmodule Visindigo
	*/

	/*!
		\enum Visindigo::Utility::FileUtility::BinarySizeFormat
		\brief 此枚举用于表示文件大小的单位
		\since Yayin Story Studio 0.13.0
		\value IEC 二进制单位
		\value SI 十进制单位
	*/

	/*!
		\enum Visindigo::Utility::FileUtility::BinarySizeUnit
		\brief 此枚举用于表示文件大小的单位
		\since Yayin Story Studio 0.13.0
		\value bit 位
		\value Byte 字节
		\value Char 字符，与Byte相同
	*/

	/*!
		\enum Visindigo::Utility::FileUtility::CountingUnit
		\brief 此枚举用于表示文件大小的单位
		\since Yayin Story Studio 0.13.0
		\value _0 （无单位）
		\value K 千
		\value M 兆
		\value G 吉
		\value T 太
		\value P 拍
		\value E 艾
		\value Z 泽
		\value Y 尧
	*/

	/*!
		\a filePath 文件路径
		\since Yayin Story Studio 0.13.0
		读取文件的每一行，并返回一个QStringList
	*/
	QStringList FileUtility::readLines(const QString& filePath) {
		QFile file(filePath);
		if (!file.exists()) {
			return QStringList();
		}
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			return QStringList();
		}
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		QStringList rtn;
		while (!ts.atEnd()) {
			rtn.append(ts.readLine());
		}
		file.close();
		return rtn;
	}
	/*!
		\a filePath 文件路径
		\since Yayin Story Studio 0.13.0
		读取文件的所有内容，并返回一个QString
	*/
	QString FileUtility::readAll(const QString& filePath) {
		QFile file(filePath);
		if (!file.exists()) {
			return QString();
		}
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			return QString();
		}
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		QString rtn = ts.readAll();
		file.close();
		return rtn;
	}
	/*!
		\a filePath 文件路径
		\since Yayin Story Studio 0.13.0
		读取文件的所有内容，并返回一个QByteArray
	*/
	QByteArray FileUtility::readByteArray(const QString& filePath) {
		QFile file(filePath);
		if (!file.exists()) {
			return QByteArray();
		}
		if (!file.open(QIODevice::ReadOnly)) {
			return QByteArray();
		}
		QByteArray rtn = file.readAll();
		file.close();
		return rtn;
	}
	/*!
		\a filePath 文件路径
		\a data 需要保存的数据
		\a joinLine 行连接符
		\since Yayin Story Studio 0.13.0
		将QStringList保存到文件中
	*/
	void FileUtility::saveLines(const QString& filePath, const QStringList& lines, const QString& joinLine) {
		QFile file(filePath);
		if (!file.exists()) {
			QFileInfo fileInfo(file);
			QString folder = fileInfo.absolutePath();
			QDir dir;
			if (!dir.exists(folder)) {
				dir.mkpath(folder);
			}
			if (!file.open(QIODevice::NewOnly | QIODevice::Text)) {
				vgErrorF << "Failed to open file, nothing saved: " << filePath;
			}
		}
		else {
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				vgErrorF << "Failed to open file, nothing saved: " << filePath;
			}
		}
		QString content = lines.join(joinLine);
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		ts << content;
		file.close();
	}
	/*!
		\a filePath 文件路径
		\a data 需要保存的数据
		\since Yayin Story Studio 0.13.0
		将QString保存到文件中
	*/
	void FileUtility::saveAll(const QString& filePath, const QString& data) {
		QFile file(filePath);
		if (!file.exists()) {
			QFileInfo fileInfo(file);
			QString folder = fileInfo.absolutePath();
			QDir dir;
			if (!dir.exists(folder)) {
				dir.mkpath(folder);
			}
			if (!file.open(QIODevice::NewOnly | QIODevice::Text)) {
				vgErrorF << "Failed to open file, nothing saved: " << filePath;
			}
		}
		else {
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				vgErrorF << "Failed to open file, nothing saved: " << filePath;
			}
		}
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		ts << data;
		file.close();
	}
	/*!
		\a filePath 文件路径
		\a data 需要保存的数据
		\since Yayin Story Studio 0.13.0
		将QByteArray保存到文件中
	*/
	void FileUtility::saveByteArray(const QString& filePath, const QByteArray& data) {
		QFile file(filePath);
		if (!file.exists()) {
			QFileInfo fileInfo(file);
			QString folder = fileInfo.absolutePath();
			QDir dir;
			if (!dir.exists(folder)) {
				dir.mkpath(folder);
			}
			if (!file.open(QIODevice::NewOnly)) {
				vgErrorF << "Failed to open file, nothing saved: " << filePath;
			}
		}
		else {
			if (!file.open(QIODevice::WriteOnly)) {
				vgErrorF << "Failed to open file, nothing saved: " << filePath;
			}
		}
		file.write(data);
		file.close();
	}

	/*!
		\a root 根目录
		\a exts 文件扩展名
		\a considerSubFolder 是否考虑子目录
		\since Yayin Story Studio 0.13.0
		过滤指定目录下的文件，返回一个QStringList。它总是返回绝对路径。

		这exts应形如 "*.ext1"、"*.ext2"、"*.ext3"，请务必注意星号。
	*/
	QStringList FileUtility::fileFilter(const QString& root, const QStringList& exts, bool considerSubFolder) {
		QDirListing list(root, exts, (considerSubFolder ? QDirListing::IteratorFlag::Recursive : QDirListing::IteratorFlag::Default) | QDirListing::IteratorFlag::FilesOnly);
		QStringList files;
		for(auto li: list){
			files.append(li.absoluteFilePath());
		}
		return files;
	}

	/*!
		\a root 根目录
		\since Yayin Story Studio 0.13.0
		获取指定目录下的文件大小，返回一个qint64，单位为字节。
	*/
	qint64 FileUtility::sizeBytes(const QString& root) {
		QDir dir(root);
		qint64 s = 0;
		for (QFileInfo fileInfo : dir.entryInfoList(QDir::Files)) {
			s += fileInfo.size();
		}
		for (QString subDir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
			s += sizeBytes(root + QDir::separator() + subDir);
		}
		return s;
	}

	/*!
		\a bytes 字节数
		\a u 单位
		\a f 格式
		\since Yayin Story Studio 0.13.0
		获取指定字节数的可读大小，返回一个QString。
	*/
	QString FileUtility::readableSize(qint64 bytes, BinarySizeUnit u, BinarySizeFormat f) {
		return FileUtilityPrivate::getReadableSize(bytes, u, CountingUnit::_0, f);
	}

	/*!
		\a path 路径
		\since Yayin Story Studio 0.13.0
		打开指定路径的资源管理器。
	*/
	void FileUtility::openExplorer(const QString& path) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}

	/*!
		\a url 链接
		\since Yayin Story Studio 0.13.0
		打开指定链接的浏览器。
	*/
	void FileUtility::openBrowser(const QString& url) {
		QDesktopServices::openUrl(QUrl(url));
	}

	/*!
		\a name 文件名
		\a replace 替换字符
		\since Yayin Story Studio 0.13.0
		将文件名中的非法字符替换为指定字符，并返回合法的文件名。
		这个函数不验证用于替换的字符是否合法，请务必保证它合法，否则可能会得到一个仍然非法的文件名。
	*/
	QString FileUtility::toLegelFileName(const QString& name, const QString& replace) {
		QString legalName = name;
		legalName.replace(QRegularExpression("[\\\\/:*?\"<>|\\s!@$]"), replace);
		return legalName;
	}

	/*!
		\a filePath 文件路径
		\since Yayin Story Studio 0.13.0
		判断指定文件是否存在，返回一个bool。
	*/
	bool FileUtility::isFileExist(const QString& filePath) {
		QFile file(filePath);
		return file.exists();
	}

	/*!
		\a dirPath 目录路径
		\since Yayin Story Studio 0.13.0
		判断指定目录是否存在，返回一个bool。
	*/
	bool FileUtility::isDirExist(const QString& dirPath) {
		QDir dir(dirPath);
		return dir.exists();
	}

	/*!
		\a dirPath 目录路径
		\since Yayin Story Studio 0.13.0
		判断指定目录是否为空，返回一个bool。
	*/
	bool FileUtility::isDirEmpty(const QString& dirPath) {
		QDir dir(dirPath);
		if (!dir.exists()) {
			return false;
		}
		return dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}

	/*!
		\a dirPath 目录路径
		\since Yayin Story Studio 0.13.0
		创建指定目录，返回一个bool，表示是否创建成功。
	*/
	bool FileUtility::createDir(const QString& dirPath) {
		QDir dir(dirPath);
		if (dir.exists()) {
			return true;
		}
		return dir.mkpath(".");
	}

	/*!
		\a startWith 起始路径
		\a path 绝对路径
		\since Yayin Story Studio 0.13.0
		如果path以startWith开头，则返回相对于startWith的路径，否则返回path。
	*/
	QString FileUtility::getRelativeIfStartWith(const QString& startWith, const QString& path) {
		if (path.startsWith(startWith)) {
			QDir dir(startWith);
			return "./" + dir.relativeFilePath(path);
		}
		else {
			return path;
		}
	}

	/*!
		\a argv 程序参数
		\since Yayin Story Studio 0.13.0
		获取程序所在目录的绝对路径，返回一个QString。
	*/
	QString FileUtility::getProgramPath(char** argv){
		if (argv==nullptr) {
			QFileInfo fileInfo(QCoreApplication::applicationFilePath());
			return fileInfo.absolutePath();
		}
		else {
			QString appPath = QString::fromUtf8(argv[0]);
			return QFileInfo(appPath).absolutePath();
		}
	}

	/*!
		\a filePath 文件路径
		\since Yayin Story Studio 0.13.0
		获取指定文件的创建时间，返回一个QDateTime。
	*/
	QDateTime FileUtility::getFileCreateTime(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		if (!fileInfo.exists()) {
			return QDateTime();
		}
		return fileInfo.birthTime();
	}

	/*!
		\a filePath 文件路径
		\since Yayin Story Studio 0.13.0
		获取指定文件的修改时间，返回一个QDateTime。
	*/
	QDateTime FileUtility::getFileModifyTime(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		if (!fileInfo.exists()) {
			return QDateTime();
		}
		return fileInfo.lastModified();
	}

	/*!
		\a filePath 文件路径
		\since Yayin Story Studio 0.13.0
		获取指定文件的访问时间，返回一个QDateTime。
	*/
	QDateTime FileUtility::getFileAccessTime(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		if (!fileInfo.exists()) {
			return QDateTime();
		}
		return fileInfo.lastRead();
	}

	/*!
		\a filePath 文件路径
		\a moveToTrash 是否移动到回收站
		\since Yayin Story Studio 0.13.0
		删除指定文件，如果moveToTrash为true，则移动到回收站，否则直接删除。
	*/
	void FileUtility::deleteFile(const QString& filePath, bool moveToTrash) {
		QFile file(filePath);
		if (!file.exists()) {
			return;
		}
		if (moveToTrash) {
			file.moveToTrash();
		}
		else {
			file.remove();
		}
	}

	/*!
		\a srcPath 源文件路径
		\a dstPath 目标文件路径
		\a rinse 是否使用漂洗的方式复制文件
		\a overwrite 是否覆盖已存在的目标文件
		\since Yayin Story Studio 0.13.0
		复制文件，如果rinse为true，则使用漂洗的方式复制文件，否则使用QFile::copy()，如果overwrite为true，则覆盖已存在的目标文件，否则不进行复制。

		漂洗模式只忠实传递文件本体的二进制数据，不从源文件读取任何元数据，也不写入任何元数据到目标文件，
		因此在某些特殊情况下可能会得到一个与源文件不同的目标文件，例如当源文件具有特殊权限时，目标文件可能会得到默认权限；
		当源文件具有特殊属性时，目标文件可能不会继承这些属性；当源文件具有特殊时间戳时，目标文件可能会得到当前时间戳。

		这对于从qrc编译到二进制文件内的资源文件向外部复制时很有用，因为qrc资源文件会自动设为只读，
		并且具有特殊的权限和属性，使用漂洗模式复制可以得到一个正常的可读写文件。
	*/
	void FileUtility::copyFile(const QString& srcPath, const QString& dstPath, bool rinse, bool overwrite) {
		QFile srcFile(srcPath);
		if (!srcFile.exists()) {
			return;
		}
		QFile dstFile(dstPath);
		if (dstFile.exists()) {
			if (overwrite) {
				dstFile.remove();
			}
			else {
				return;
			}
		}
		if (not rinse) {	
			srcFile.copy(dstPath);
		}
		else {
			srcFile.open(QIODevice::ReadOnly);
			QByteArray data = srcFile.readAll();
			srcFile.close();
			dstFile.open(QIODevice::WriteOnly);
			dstFile.write(data);
			dstFile.close();
		}
	}
}