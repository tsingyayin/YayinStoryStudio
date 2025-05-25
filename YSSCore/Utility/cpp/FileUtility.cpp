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

	QStringList FileUtility::fileFilter(const QString& root, const QStringList& exts, bool considerSubFolder) {
		QDirIterator it(root, exts, QDir::Files, considerSubFolder ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
		QStringList files;
		while (it.hasNext())
		{
			files << it.next();
		}
		return files;
	}

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

	QString FileUtility::readableSize(qint64 bytes, BinarySizeUnit u, BinarySizeFormat f) {
		return FileUtilityPrivate::getReadableSize(bytes, u, CountingUnit::_0, f);
	}

	void FileUtility::openExplorer(const QString& path) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}

	void FileUtility::openBrowser(const QString& url) {
		QDesktopServices::openUrl(QUrl(url));
	}
}