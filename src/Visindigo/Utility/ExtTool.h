#pragma once
#include "../VICompileMacro.h"
#include <QtCore/qstring.h>
// Main
namespace Visindigo::Utility {
	class VisindigoAPI ExtTool {
	public:
		static void registerFileExtMetaInfo(const QString& ext, const QString& description, const QString& iconPath = "");
		static void registerFileExtLaunchInfo(const QString& ext, const QString& command) {};
	};
}