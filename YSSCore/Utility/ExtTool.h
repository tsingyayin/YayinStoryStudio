#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>
// Main
namespace YSSCore::Utility {
	class YSSCoreAPI ExtTool {
	public:
		static void registerFileExtension(const QString& ext, const QString& description, const QString& iconPath = "");
	};
}