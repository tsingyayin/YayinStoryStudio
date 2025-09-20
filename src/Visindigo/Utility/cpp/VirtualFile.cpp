#include "../VirtualFile.h"
#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmap.h>
#include <QtCore/qdir.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qrandom.h>
namespace Visindigo::Utility {
	class VirtualFilePrivate
	{
		friend class VirtualFile;
	protected:
		static VirtualFile* Instance;
		QMap<QString, QByteArray> FileDataMap;
		QString SavePath;
		void clearFiles() {
			QDir dir(SavePath);
			dir.removeRecursively();
		}
	};
	VirtualFile* VirtualFilePrivate::Instance = nullptr;
	VirtualFile::VirtualFile(const QString& savePath) {
		d = new VirtualFilePrivate;
		VirtualFilePrivate::Instance = this;
		changeSavePath(savePath);
	}
	VirtualFile* VirtualFile::getInstance() {
		return VirtualFilePrivate::Instance;
	}
	VirtualFile::~VirtualFile() {
		d->clearFiles();
		delete d;
	}
	void VirtualFile::changeSavePath(const QString& savePath) {
		d->clearFiles();
		if (savePath.isEmpty()) {
			d->SavePath = QDir::tempPath() + "/yayinstorystudio";
		}
		else {
			d->SavePath = savePath;
		}
	}
	bool VirtualFile::createTextFile(const QString& filePath, const QString& content) {
		if (d->FileDataMap.contains(filePath)) {
			return false;
		}
		d->FileDataMap[filePath] = content.toUtf8();
		return true;
	}
	bool VirtualFile::createBinaryFile(const QString& filePath, const QByteArray& content) {
		if (d->FileDataMap.contains(filePath)) {
			return false;
		}
		d->FileDataMap[filePath] = content;
		return true;
	}
	bool VirtualFile::deleteFile(const QString& filePath) {
		if (d->FileDataMap.contains(filePath)) {
			d->FileDataMap.remove(filePath);
			return true;
		}
		return false;
	}
	QString VirtualFile::file(const QString& filePath) {
		if (!d->FileDataMap.contains(filePath)) {
			return "";
		}
		QFile file(d->SavePath + filePath);
		if (file.exists()) {
			return d->SavePath + filePath;
		}
		file.open(QIODevice::NewOnly);
		file.write(d->FileDataMap[filePath]);
		file.close();
		return d->SavePath + filePath;
	}
}