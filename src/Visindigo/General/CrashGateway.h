#ifndef Visindigo_General_CrashGateway_h
#define Visindigo_General_CrashGateway_h
#include <QtCore/qstring.h>
#include "VICompileMacro.h"

// Forward declarations
namespace Visindigo::General {
	class Exception;
}

// Main
namespace Visindigo::General {
	class VisindigoAPI CrashGateway final {
	public:
		static void install();
		static void uninstall();
		static bool installed();
		static bool handling() noexcept;
	public:
		static QString getReportFolderPath();
		static void setLogFolder(const QString& folder);
		static void setLogFileName(const QString& name);
		static void setDumpEnabled(bool enabled);
		static bool isDumpEnabled();
		static void setMiniDumpType(quint32 type);
		static quint32 getMiniDumpType();
		static quint32 defaultMiniDumpType();
		static void setSymbolResolutionEnabled(bool enabled);
		static bool isSymbolResolutionEnabled();
		static void setProductInfo(const QString& info);
		static void setHardwareInfo(const QString& info);
	public:
		static void onCaughtException(const Exception& ex);
	};
}
#endif // Visindigo_General_CrashGateway_h
