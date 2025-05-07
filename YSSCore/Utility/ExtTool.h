#pragma once
#include <QString>
#include "../Macro.h"

namespace YSSCore::Utility {
	class YSSCoreAPI ExtTool {
	public:
		static void registerFileExtension(const QString& ext, const QString& description);
	};
}