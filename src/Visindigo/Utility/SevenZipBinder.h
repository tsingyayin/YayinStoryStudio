#ifndef Visindigo_Utility_SevenZipBinder_h
#define Visindigo_Utility_SevenZipBinder_h
#include "VICompileMacro.h"
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
namespace Visindigo::Utility {
	class SevenZipBinderPrivate;
	class VisindigoAPI SevenZipBinder :public QObject {
		Q_OBJECT;
	public:
		enum CompressFormat {
			__Both__ = 0,
			sevenz, xz, bzip2, gzip, tar, zip, wim,
			__DecompOnly__ = 1000,
			ar, arj, cab, chm, cpio, cramfs, dmg, ext, fat, gpt,
			hfs, ihex, iso, lzh, lzma, mbr, msi, nsis, ntfs, qcow2, rar, rpm, 
			squashfs, udf, vhd, vmdk, xar, z, zstd
		};
		Q_ENUM(CompressFormat)
	signals:
		void processing(const QString& fileName, float percent);
		void processed(bool success);
	private:
		SevenZipBinder();
	public:
		static SevenZipBinder* getInstance();
		~SevenZipBinder();
		void bind7zaBinary(const QString& exePath);
		QString get7zaBinaryPath() const;
		bool compressFilesToZip(const QStringList& filePaths, const QString& arcPath, CompressFormat format = zip, const QString& password = ""); // filePaths could be one folder, will compress the whole folder
		bool decompressFile(const QString& arcPath, const QString& outputPath, CompressFormat format = zip, const QString& password = "");
		bool isBusy() const;
	private:
		SevenZipBinderPrivate* d;
	};
}

#define VI7zBinder Visindigo::Utility::SevenZipBinder::getInstance()
#endif // Visindigo_Utility_SevenZipBinder_h