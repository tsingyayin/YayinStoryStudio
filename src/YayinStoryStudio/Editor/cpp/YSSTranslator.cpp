#include "../YSSTranslator.h"

namespace YSS::Editor {
	YSSTranslator::YSSTranslator(Visindigo::General::Plugin* parent) : Translator(parent, "YSS") {
		setDefaultLang(zh_CN);
		setLangFilePath({
			{ zh_CN, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/zh_CN.json" },
			{ en, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/en.json" }
			});
	}
}