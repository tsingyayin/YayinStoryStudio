#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

class QByteArray;

namespace YSSCore::Utility {
	class YSSCoreAPI FileUtility {
	public:
		enum BinarySizeFormat { // The number of this enum can't be changed, as it is used in calculation directly
			IEC = 1024,
			SI = 1000,
		};
		enum BinarySizeUnit { // The number of this enum can't be changed, as it is used in calculation directly
			bit = 1,
			Byte = 8,
			Char = Byte
		};
		enum CountingUnit { // The number of this enum can't be changed, as it is used in calculation directly
			_0 = 1, K, M, G, T, P, E, Z, Y
		};
	public:
		static QStringList readLines(const QString& filePath);
		static QString readAll(const QString& filePath);
		static QByteArray readByteArray(const QString& filePath);
		static void saveLines(const QString& filePath, const QStringList& data, const QString& joinLine = "\n");
		static void saveAll(const QString& filePath, const QString& data);
		static void saveByteArray(const QString& filePath, const QByteArray& data);
		static QStringList fileFilter(const QString& root, const QStringList& exts, bool considerSubFolder = true);
		static qint64 sizeBytes(const QString& root);
		static QString readableSize(qint64 bytes, BinarySizeUnit u = Byte, BinarySizeFormat f = IEC);
		static void openExplorer(const QString& path);
		static void openBrowser(const QString& url);
	};
}