#pragma once
#include <QtCore/qstring.h>
#include "../Macro.h"
// Forward declarations
class QByteArray;
namespace YSSCore::Utility {
	class VirtualFilePrivate;
}
// Main
namespace YSSCore::Utility {
	class YSSCoreAPI VirtualFile {
	public:
		VirtualFile(const QString& savePath = "");
		~VirtualFile();
		void changeSavePath(const QString& savePath);
		static VirtualFile* getInstance();
		bool createTextFile(const QString& filePath, const QString& content);
		bool createBinaryFile(const QString& filePath, const QByteArray& content);
		bool deleteFile(const QString& filePath);
		QString file(const QString& filePath);
	private:
		VirtualFilePrivate* d;
	};
	using TempFile = VirtualFile;
}
// Global Macros
#define YSSVirtualFile YSSCore::Utility::VirtualFile::getInstance()->file