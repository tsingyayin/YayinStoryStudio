#pragma once
#include "../Macro.h"

class QString;
namespace YSSCore::Utility {
	class YSSCoreAPI ExtTool {
	public:
		static void registerFileExtension(const QString& ext, const QString& description);
	};
}