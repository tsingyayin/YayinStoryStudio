#ifndef YayinStoryStudio_Editor_YSSTranslator_h
#define YayinStoryStudio_Editor_YSSTranslator_h
#include <General/Translator.h>

namespace YSS::Editor {
	class YSSTranslator :public Visindigo::General::Translator {
	public:
		YSSTranslator(Visindigo::General::Plugin* parent);
	};
}
#endif // YayinStoryStudio_Editor_YSSTranslator_h
