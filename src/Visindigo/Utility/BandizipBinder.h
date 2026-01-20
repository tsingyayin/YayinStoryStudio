#ifndef Visindigo_Utility_BandizipBinder_h
#define Visindigo_Utility_BandizipBinder_h
/*
	NOTICE: 
Even though the standard version of Bandizip is free and allows use in any scenario, 
whether for profit or non-profit, we seem to have not found any distribution regulations regarding its binary content. 
Therefore, it is absolutely impossible for Visindigo to directly provide a copy containing binaries. 

However, you can easily assign this Binder class to the corresponding location of the Bandizip software. 
Using it is much faster than compiling and integrating those libraries.

* To enable users to use it out of the box, the Bandizip binary path is set to the current working directory by default.
* Default binary name is "bz.exe". Cross platform support is not considered for now.
*/
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
namespace Visindigo::Utility {
	class BandizipBinderPrivate;
	class BandizipBinder :public QObject {
		Q_OBJECT;
	public:
		enum CompressFormat {
				zip, zipx, exe, tar, tgz,
				lzh, iso, sevenz, gz, xz
		};
	signals:
		void compressingFile(const QString& filePath);
		void compressedFile(const QString& filePath);
		void decompressingFile(const QString& filePath);
		void decompressedFile(const QString& folderPath);
	private:
		BandizipBinder();
	public:
		static BandizipBinder* getInstance();
		~BandizipBinder();
		void bindBandizipBinary(const QString& bandizipExePath);
		QString getBandizipBinaryPath() const;
		bool compressFilesToZip(const QStringList& filePaths, const QString& arcPath, CompressFormat format = zip, const QString& password = ""); // filePaths could be one folder, will compress the whole folder
		bool decompressFile(const QString& arcPath, const QString& outputPath, CompressFormat format = zip, const QString& password = "");
		bool isBusy() const;
	private:
		BandizipBinderPrivate* d;
	};
}

#define VIBandizipBinder Visindigo::Utility::BandizipBinder::getInstance()
#endif // Visindigo_Utility_BandizipBinder_h