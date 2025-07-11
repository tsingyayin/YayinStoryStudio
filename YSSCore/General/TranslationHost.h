#pragma once
#include "../Macro.h"
#include "Translator.h"
class QString;

namespace YSSCore::General {
	class Translator;
	class TranslationHostPrivate;
	class YSSCoreAPI TranslationHost {
	public:
		static TranslationHost* getInstance();
		TranslationHost();
		~TranslationHost();
		void setLangID(Translator::LangID id);
		void active(Translator* translator);
		QString tr(const QString& nameSpace, const QString& key);
		QString tr(const QString& key);
		QString i18n(const QString& raw);
	private:
		TranslationHostPrivate* d;
	};
}

#define YSSTR YSSCore::General::TranslationHost::getInstance()->tr
#define YSSI18N YSSCore::General::TranslationHost::getInstance()->i18n