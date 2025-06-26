#include "../FileUtility.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qdiriterator.h>
#include <QtGui/qdesktopservices.h>
#include <QtCore/qurl.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qcoreapplication.h>

#define VI_ENUMSTR(enumName, enumValue) case enumName::enumValue: return #enumValue;

namespace YSSCore::Utility {
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
		\class YSSCore::Utility::FileUtility
		\brief 此类为Yayin Story Studio 提供文件操作的相关函数
		\since Yayin Story Studio 0.13.0
		\inmodule YSSCore
	*/

	/*!
		\enum YSSCore::Utility::FileUtility::BinarySizeFormat
		\brief 此枚举用于表示文件大小的单位
		\since Yayin Story Studio 0.13.0
		\value IEC 二进制单位
		\value SI 十进制单位
	*/

	/*!
		\enum YSSCore::Utility::FileUtility::BinarySizeUnit
		\brief 此枚举用于表示文件大小的单位
		\since Yayin Story Studio 0.13.0
		\value bit 位
		\value Byte 字节
		\value Char 字符，与Byte相同
	*/

	/*!
		\enum YSSCore::Utility::FileUtility::CountingUnit
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
		file.open(QIODevice::ReadOnly | QIODevice::Text);
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
		file.open(QIODevice::ReadOnly | QIODevice::Text);
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
		file.open(QIODevice::ReadOnly);
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
			file.open(QIODevice::NewOnly | QIODevice::Text);
		}
		else {
			file.open(QIODevice::WriteOnly | QIODevice::Text);
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
			file.open(QIODevice::NewOnly | QIODevice::Text);
		}
		else {
			file.open(QIODevice::WriteOnly | QIODevice::Text);
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
			file.open(QIODevice::NewOnly);
		}
		else {
			file.open(QIODevice::WriteOnly);
		}
		file.write(data);
		file.close();
	}

	/*!
		\a root 根目录
		\a exts 文件扩展名
		\a considerSubFolder 是否考虑子目录
		\since Yayin Story Studio 0.13.0
		过滤指定目录下的文件，返回一个QStringList。

		这exts应形如 "*.ext1"、"*.ext2"、"*.ext3"，请务必注意星号。
	*/
	QStringList FileUtility::fileFilter(const QString& root, const QStringList& exts, bool considerSubFolder) {
		QDirIterator it(root, exts, QDir::Files, considerSubFolder ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
		QStringList files;
		while (it.hasNext())
		{
			files << it.next();
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

	QString FileUtility::toLegelFileName(const QString& name, const QString& replace) {
		QString legalName = name;
		legalName.replace(QRegularExpression("[\\\\/:*?\"<>|\\s!@$]"), replace);
		return legalName;
	}

	bool FileUtility::isFileExist(const QString& filePath) {
		QFile file(filePath);
		return file.exists();
	}

	bool FileUtility::isDirExist(const QString& dirPath) {
		QDir dir(dirPath);
		return dir.exists();
	}

	bool FileUtility::isDirEmpty(const QString& dirPath) {
		QDir dir(dirPath);
		if (!dir.exists()) {
			return false;
		}
		return dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}

	bool FileUtility::createDir(const QString& dirPath) {
		QDir dir(dirPath);
		if (dir.exists()) {
			return true;
		}
		return dir.mkpath(".");
	}

	QString FileUtility::getRelativeIfStartWith(const QString& startWith, const QString& path) {
		if (path.startsWith(startWith)) {
			QDir dir(startWith);
			return "./" + dir.relativeFilePath(path);
		}
		else {
			return path;
		}
	}

	QString FileUtility::getProgramPath() {
		QFileInfo fileInfo(QCoreApplication::applicationFilePath());
		return fileInfo.absolutePath();
	}
}