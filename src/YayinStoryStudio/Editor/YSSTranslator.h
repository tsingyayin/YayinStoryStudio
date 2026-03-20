#pragma once
#include <General/Translator.h>

namespace YSS::Editor {
	class YSSTranslator :public Visindigo::General::Translator {
	public:
		YSSTranslator(Visindigo::General::Plugin* parent);
	};
}