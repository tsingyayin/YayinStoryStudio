#pragma once
#include "../Macro.h"

class QString;
namespace YSSCore::Utility {
	class StringMacroProvider;
	VIAPIClass(StringMacro) {
		VImpl(StringMacro);
		VI_Singleton(StringMacro);
		StringMacro();
		void addProvider(StringMacroProvider * provider);
		QString parse(QString str) const;
	};
}

#define YSSSMParse YSSCore::Utility::StringMacro::getInstance()->parse