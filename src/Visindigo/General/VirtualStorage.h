#ifndef Visindigo_Utility_VirtualStorage_h
#define Visindigo_Utility_VirtualStorage_h
#include "VICompileMacro.h"
#include <QtCore/qstring.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>
#include "Utility/JsonConfig.h"
namespace Visindigo::Utility {
	class VirtualStoragePrivate;
	class VisindigoAPI VirtualStorage {
	public:
		VirtualStorage(const QString& dbPath);
		~VirtualStorage();
		void saveFile(const QString& path, const QByteArray& data);
		QByteArray readFile(const QString& path);
		QByteArray readFileByVID(const QString& vid);
		qint32 getFileSize(const QString& path);
		qint32 getFileRepeatCount(const QString& path);
		QString getFileVID(const QString& path);
		QString removeFile(const QString& path);
		QString removeFileByVID(const QString& vid);
		JsonConfig listFolder(const QString& folderPath, bool recursive = false);
	private:
		VirtualStoragePrivate* d;
	};
}
#endif // Visindigo_Utility_VirtualStorage_h