#include "../ASEDevTranslator.h"
ASEDevTranslator::ASEDevTranslator(Visindigo::General::Plugin* parent) :
	Visindigo::General::Translator(parent, "ASEDevTool")
{
	setDefaultLang(zh_CN);
	setLangFilePath({
		{zh_CN, ":/resource/cn.yxgeneral.ase_dev_tool/i18n/zh_CN.json"},
		{en_US, ":/resource/cn.yxgeneral.ase_dev_tool/i18n/en_US.json"},
		});
}