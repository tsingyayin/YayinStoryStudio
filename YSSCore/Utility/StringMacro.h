#pragma once
#include "../Macro.h"
// Forward declarations
class QString;
namespace YSSCore::Utility {
	class StringMacroProvider;
}
// Main
namespace YSSCore::Utility {
	VIAPIClass(StringMacro) {
		VImpl(StringMacro);
		VI_Singleton(StringMacro);
		StringMacro();
		void addProvider(StringMacroProvider * provider);
		QString parse(QString str) const;
	};
}
// Global Macros
#define YSSSMParse YSSCore::Utility::StringMacro::getInstance()->parse