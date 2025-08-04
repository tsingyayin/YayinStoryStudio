#pragma once
#include "../Macro.h"
#include "Translator.h"
class QString;

namespace Visindigo::General {
	class Translator;
	class TranslationHostPrivate;
	class VisindigoAPI TranslationHost {
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

#define YSSTR Visindigo::General::TranslationHost::getInstance()->tr
#define YSSI18N Visindigo::General::TranslationHost::getInstance()->i18n