#include "Editor/YSSTranslator.h"

namespace YSS::Editor {
	YSSTranslator::YSSTranslator(Visindigo::General::Plugin* parent) : Translator(parent, "YSS") {
		setDefaultLang(zh_CN);
		setLangFilePath({
			{ zh_CN, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/zh_CN.json" },
			{ zh_TW, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/zh_TW.json" },
			{ en, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/en.json" },
			{ ja, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/ja.json" },
			{ jp_less_loanword, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/jp_less_loanword.json" },
			{ ko, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/ko.json" },
			{ ru, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/ru.json" },
			{ de, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/de.json" },
			{ fr, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/fr.json" }
			});
	}
}