#include "Editor/YSSTranslator.h"

namespace YSS::Editor {
	YSSTranslator::YSSTranslator(Visindigo::General::Plugin* parent) : Translator(parent, "YSS") {
		setDefaultLang(zh_CN);
		setLangFilePath({
			{ zh_CN, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/zh_CN.json" },
			{ en, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/en.json" },
			{ ja, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/ja.json" },
			{ ko, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/ko.json" },
			{ ru, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/ru.json" },
			{ de, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/de.json" },
			{ fr, ":/resource/cn.yxgeneral.yayinstorystudio/i18n/fr.json" }
			});
	}
}