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
		\since Visindigo 0.13.0
		\inmodule Visindigo

		这类所有函数都是静态函数，所以你不需要创建它的实例。

		总的来说，这里都是一些仅通过QDir、QFile、QFileInfo等文件系统相关类
		实现的函数。
	*/

	/*!
		\enum Visindigo::Utility::FileUtility::BinarySizeFormat
		\brief 此枚举用于表示文件大小的单位
		\since Visindigo 0.13.0
		\value IEC 二进制单位
		\value SI 十进制单位

		以IEC为单位的文件大小单位是基于1024的，而以SI为单位的文件大小单位是基于1000的。
		前者会产生KiB、MiB、GiB等单位，而后者会产生KB、MB、GB等单位。
		请注意，虽然在日常使用中我们经常将KB、MB等单位误用来表示基于1024的单位，
		但实际上它们应该是基于1000的单位，这是微软的黑锅。
	*/

	/*!
		\enum Visindigo::Utility::FileUtility::BinarySizeUnit
		\brief 此枚举用于表示文件大小的单位
		\since Visindigo 0.13.0
		\value bit 位
		\value Byte 字节
		\value Char 字符，与Byte相同
	*/

	/*!
		\enum Visindigo::Utility::FileUtility::CountingUnit
		\brief 此枚举用于表示文件大小的单位
		\since Visindigo 0.13.0
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
		\since Visindigo 0.13.0
		\a filePath 文件路径

		return 以行列表的形式读取文件的内容。如果文件不存在或无法打开，则返回一个空列表。
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
		\since Visindigo 0.13.0
		\a filePath 文件路径

		读取文件的所有内容。如果文件不存在或无法打开，则返回一个空字符串。
		return 包含所有文本内容的QString。
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
		\since Visindigo 0.13.0
		\a filePath 文件路径

		读取文件的所有内容。如果文件不存在或无法打开，则返回一个空QByteArray。
		return 包含所有内容的QByteArray。
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
		\since Visindigo 0.13.0
		\a filePath 文件路径
		\a data 需要保存的数据
		\a joinLine 行连接符

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
		\since Visindigo 0.13.0
		\a filePath 文件路径
		\a data 需要保存的数据

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
		\since Visindigo 0.13.0
		\a filePath 文件路径
		\a data 需要保存的数据
		
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
		\since Visindigo 0.13.0
		\a root 根目录
		\a exts 文件扩展名，这exts应形如 "*.ext1"、"*.ext2"、"*.ext3"，请务必注意星号。
		\a considerSubFolder 是否考虑子目录
		
		过滤指定目录下的文件。

		return 一个QStringList，包含了所有符合条件的文件的绝对路径。
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
		\since Visindigo 0.14.0
		\a root 根目录
		\a considerSubFolder 是否考虑子目录。请注意，这个函数的considerSubFolder默认为false，与fileFilter不同。

		获取指定目录下的文件夹。

		return 一个QStringList，包含了所有符合条件的文件夹的绝对路径。
	*/
	QStringList FileUtility::folderFilter(const QString& root, bool considerSubFolder) {
		QDirListing list(root, QStringList(), (considerSubFolder ? QDirListing::IteratorFlag::Recursive : QDirListing::IteratorFlag::Default) | QDirListing::IteratorFlag::DirsOnly);
		QStringList folders;
		for(auto li: list){
			folders.append(li.absoluteFilePath());
		}
		return folders;
	}

	/*!
		\since Visindigo 0.13.0
		\a root 根目录

		return 指定目录下的文件大小，单位为字节。如果目录不存在，则返回0。
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
		\since Visindigo 0.13.0
		\a bytes 字节数
		\a u 单位
		\a f 格式
		
		获取指定字节数的可读大小，
		return 一个QString，包含了可读大小和单位，例如"1.23MB"。
	*/
	QString FileUtility::readableSize(qint64 bytes, BinarySizeUnit u, BinarySizeFormat f) {
		return FileUtilityPrivate::getReadableSize(bytes, u, CountingUnit::_0, f);
	}

	/*!
		\since Visindigo 0.13.0
		\a path 路径
	
		打开指定路径的资源管理器。
		从0.15.0开始，这个函数在Windows上进行优化：如果路径是一个文件，则会额外选中该文件。
	*/
	void FileUtility::openExplorer(const QString& path) {
#ifndef Q_OS_WIN
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
#else
		QFileInfo fileInfo(path);
		if (fileInfo.isDir()) {
			QDesktopServices::openUrl(QUrl::fromLocalFile(path));
		}
		else if (fileInfo.isFile()) {
			QString absPath = fileInfo.absoluteFilePath().replace("/","\\");
			QString cmd = "explorer /select,\"" + absPath + "\"";
			vgDebug << cmd;
			system(("explorer /select,\"" + absPath +"\"").toStdString().c_str());
		}
#endif
	}

	/*!
		\since Visindigo 0.13.0
		\a url 链接
		
		打开指定链接的浏览器。
	*/
	void FileUtility::openBrowser(const QString& url) {
		QDesktopServices::openUrl(QUrl(url));
	}

	/*!
		\since Visindigo 0.13.0
		\a name 文件名
		\a replace 替换字符
		
		将文件名中的非法字符替换为指定字符
		return 返回合法的文件名。

		这个函数不验证用于替换的字符是否合法，请务必保证它合法，否则可能会得到一个仍然非法的文件名。

		\warning 这个函数的名称存在拼写错误，预备于0.17.0之后修正，且不保留旧名称的兼容性。
	*/
	QString FileUtility::toLegelFileName(const QString& name, const QString& replace) {
		QString legalName = name;
		legalName.replace(QRegularExpression("[\\\\/:*?\"<>|\\s!@$]"), replace);
		return legalName;
	}

	/*!
		\since Visindigo 0.13.0
		\a filePath 文件路径
		
		return 指定文件是否存在
	*/
	bool FileUtility::isFileExist(const QString& filePath) {
		QFile file(filePath);
		return file.exists();
	}

	/*!
		\since Visindigo 0.13.0
		\a dirPath 目录路径
		
		return 指定目录是否存在
	*/
	bool FileUtility::isDirExist(const QString& dirPath) {
		QDir dir(dirPath);
		return dir.exists();
	}

	/*!
		\since Visindigo 0.13.0
		\a dirPath 目录路径
		
		return 指定目录是否为空。如果目录不存在，则返回false。
	*/
	bool FileUtility::isDirEmpty(const QString& dirPath) {
		QDir dir(dirPath);
		if (!dir.exists()) {
			return false;
		}
		return dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}

	/*!
		\since Visindigo 0.13.0
		\a dirPath 目录路径
		
		创建指定目录，
		
		return 是否创建成功。
	*/
	bool FileUtility::createDir(const QString& dirPath) {
		QDir dir(dirPath);
		if (dir.exists()) {
			return true;
		}
		return dir.mkpath(".");
	}

	/*!
		\since Visindigo 0.13.0
		\a startWith 起始路径
		\a path 绝对路径
		
		return 如果path以startWith开头，相对于startWith的路径，否则返回path。
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
		\since Visindigo 0.13.0
		\a argv 程序启动时输入main的参数
		
		return 程序所在目录的绝对路径。
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
		\since Visindigo 0.13.0
		\a filePath 文件路径
		
		return 指定文件的创建时间，返回一个QDateTime。如果文件不存在，则返回一个无效的QDateTime。
	*/
	QDateTime FileUtility::getFileCreateTime(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		if (!fileInfo.exists()) {
			return QDateTime();
		}
		return fileInfo.birthTime();
	}

	/*!
		\since Visindigo 0.13.0
		\a filePath 文件路径
		
		return 指定文件的修改时间，返回一个QDateTime。如果文件不存在，则返回一个无效的QDateTime。
	*/
	QDateTime FileUtility::getFileModifyTime(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		if (!fileInfo.exists()) {
			return QDateTime();
		}
		return fileInfo.lastModified();
	}

	/*!
		\since Visindigo 0.13.0
		\a filePath 文件路径
		
		return 指定文件的访问时间，返回一个QDateTime。如果文件不存在，则返回一个无效的QDateTime。
	*/
	QDateTime FileUtility::getFileAccessTime(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		if (!fileInfo.exists()) {
			return QDateTime();
		}
		return fileInfo.lastRead();
	}

	/*!
		\since Visindigo 0.13.0
		\a filePath 文件路径
		\a moveToTrash 是否移动到回收站
		
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
		\since Visindigo 0.13.0
		\a srcPath 源文件路径
		\a dstPath 目标文件路径
		\a rinse 是否使用漂洗的方式复制文件
		\a overwrite 是否覆盖已存在的目标文件
		
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