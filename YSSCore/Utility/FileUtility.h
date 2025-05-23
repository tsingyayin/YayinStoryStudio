#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

class QByteArray;

namespace YSSCore::Utility {
	class YSSCoreAPI FileUtility {
	public:
		static QStringList readLines(const QString& filePath);
		static QString readAll(const QString& filePath);
		static QByteArray readByteArray(const QString& filePath);
		static void saveLines(const QString& filePath, const QStringList& data, const QString& joinLine = "\n");
		static void saveAll(const QString& filePath, const QString& data);
		static void saveByteArray(const QString& filePath, const QByteArray& data);
	};
}