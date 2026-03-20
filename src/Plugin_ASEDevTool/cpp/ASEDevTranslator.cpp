#include "../ASEDevTranslator.h"
ASEDevTranslator::ASEDevTranslator(Visindigo::General::Plugin* parent) :
	Visindigo::General::Translator(parent, "ASEDevTool")
{
	setDefaultLang(zh_CN);
	setLangFilePath({
		{zh_CN, ":/plugin/compiled/ASEDevTool/i18n/zh_CN.json"},
		{en_US, ":/plugin/compiled/ASEDevTool/i18n/en_US.json"},
		});
}