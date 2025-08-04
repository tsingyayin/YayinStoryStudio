#pragma once
#include "../Macro.h"
#include <QtCore/qstring.h>
// Main
namespace Visindigo::Utility {
	class VisindigoAPI ExtTool {
	public:
		static void registerFileExtension(const QString& ext, const QString& description, const QString& iconPath = "");
	};
}