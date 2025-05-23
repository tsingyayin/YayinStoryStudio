#include "../FileUtility.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <QtCore/qtextstream.h>

namespace YSSCore::Utility {
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
}