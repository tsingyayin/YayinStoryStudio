#pragma once
#include <General/Translator.h>

class ASEDevTranslator :public YSSCore::General::Translator {
	static ASEDevTranslator* Instance;
public:
	static ASEDevTranslator* getInstance();
	ASEDevTranslator();
};