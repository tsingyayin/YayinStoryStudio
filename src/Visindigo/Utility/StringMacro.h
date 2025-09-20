#pragma once
#include "../Macro.h"
// Forward declarations
class QString;
namespace Visindigo::Utility {
	class StringMacroProvider;
}
// Main
namespace Visindigo::Utility {
	VIAPIClass(StringMacro) {
		VImpl(StringMacro);
		VI_Singleton(StringMacro);
		StringMacro();
		void addProvider(StringMacroProvider * provider);
		QString parse(QString str) const;
	};
}
// Global Macros
#define YSSSMParse Visindigo::Utility::StringMacro::getInstance()->parse