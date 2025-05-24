#include "../ASEDevTranslator.h"

ASEDevTranslator* ASEDevTranslator::Instance = nullptr;

ASEDevTranslator* ASEDevTranslator::getInstance() {
	return Instance;
}
ASEDevTranslator::ASEDevTranslator() {
	Instance = this;
	setNamespace("ASEDevTool");
	setDefaultLang(zh_CN);
	setLangFilePath({
		{zh_CN, ":/plugin/compiled/ASEDevTool/i18n/zh_CN.json"},
		{en_US, ":/plugin/compiled/ASEDevTool/i18n/en_US.json"},
		});
}