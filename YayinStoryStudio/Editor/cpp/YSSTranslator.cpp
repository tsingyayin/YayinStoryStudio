#include "../YSSTranslator.h"

namespace YSS::Editor {
	YSSTranslator* YSSTranslator::Instance = nullptr;
	YSSTranslator::YSSTranslator() {
		Instance = this;
		setNamespace("YSS");
		setDefaultLang(zh_CN);
		setLangFilePath({
			{ zh_CN, ":/yss/compiled/i18n/zh_CN.json" },
			{ en_US, ":/yss/compiled/i18n/en_US.json" }
			});
	}
}