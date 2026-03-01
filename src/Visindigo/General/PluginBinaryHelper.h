#ifndef Visindigo_General_PluginBinaryHelper_h
#define Visindigo_General_PluginBinaryHelper_h
#include "../Macro.h"
#include <QtCore/qobject.h>
namespace Visindigo::General {
	class PluginBinaryHelperPrivate;
	class PluginBinaryHelper :public QObject {
		Q_OBJECT;
	public:
		enum PlatformUnit {
			Windows = 0x0001,
			Linux = 0x0002,
			MacOS = 0x0004,
			Android = 0x0008,
			iOS = 0x0010,
			OtherOS = 0x0020,
			x86_32 = 0x0100,
			x86_64 = 0x0200,
			AMD64 = x86_64,
			ARM64 = 0x0400,
			OtherArch = 0x0800,
			MSVC = 0x1000,
			GCC = 0x2000,
			Clang = 0x4000,
			OtherCompiler = 0x8000,

			// Common combinations
			Windows_AMD64 = Windows | AMD64 | MSVC,
			Windows_ARM64 = Windows | ARM64 | MSVC,
			Linux_x86_32 = Linux | x86_32 | GCC,
			Linux_x86_64 = Linux | x86_64 | GCC,
			Linux_ARM64 = Linux | ARM64 | GCC,
			MacOS_x86_64 = MacOS | x86_64 | Clang,
			MacOS_ARM64 = MacOS | ARM64 | Clang,
			Android_ARM64 = Android | ARM64 | Clang,
			iOS_ARM64 = iOS | ARM64 | Clang
		};
		Q_DECLARE_FLAGS(Platform, PlatformUnit);
		enum LoadResult {
			Unknown = 0,
			Success = 1,
			InvalidFileType = 2,
			InvalidLength = 3,
		};
	public:
		PluginBinaryHelper(QObject* parent = nullptr) {};
		~PluginBinaryHelper() {};
	public:
		void setBinaryFilePath(const QString& path);
		bool load();
		bool save(const QString& outputPath = QString());
		QList<Platform> getSupportedPlatforms() const;
		QStringList getDependencies() const;
		QString getPluginID() const;
		QString derivePluginForPlatform(Platform platform, const QString& outputPath) const;
		bool mergePlatformBinary(Platform platform, const QString& binaryPath);
		bool isCompressed() const;
		void setCompressed(bool compressed);
	};
}
#endif // Visindigo_General_PluginBinaryHelper_h