#pragma once
#include <General/Translator.h>

class ASEDevTranslator :public Visindigo::General::Translator {
	static ASEDevTranslator* Instance;
public:
	static ASEDevTranslator* getInstance();
	ASEDevTranslator();
};