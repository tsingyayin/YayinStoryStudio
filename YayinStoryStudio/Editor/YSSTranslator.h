#pragma once
#include <General/Translator.h>

namespace YSS::Editor {
	class YSSTranslator :public Visindigo::General::Translator {
		static YSSTranslator* Instance;
	public:
		static YSSTranslator* getInstance();
		YSSTranslator();
	};
}