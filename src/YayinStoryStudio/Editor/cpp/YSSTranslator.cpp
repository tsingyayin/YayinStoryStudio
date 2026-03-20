#include "../YSSTranslator.h"

namespace YSS::Editor {
	YSSTranslator::YSSTranslator(Visindigo::General::Plugin* parent) : Translator(parent, "YSS") {
		setDefaultLang(zh_CN);
		setLangFilePath({
			{ zh_CN, ":/yss/compiled/i18n/zh_CN.json" },
			{ en_US, ":/yss/compiled/i18n/en_US.json" }
			});
	}
}