#include "../TranslationHost.h"
#include "../private/Translator_p.h"

namespace YSSCore::General {
	class TranslationHostPrivate {
		friend class TranslationHost;
		static TranslationHost* Instance;
		QMap<QString, Translator*> Translators;
		Translator::LangID GlobalID = Translator::LangID::zh_CN;
		void refreshLang(Translator::LangID langID) {
			for (QString id : Translators.keys()) {
				Translator* trans = Translators[id];
				trans->d->loadTranslationFile(langID);
			}
		}
	};
	TranslationHost* TranslationHostPrivate::Instance = nullptr;
	TranslationHost::TranslationHost() {
		d = new TranslationHostPrivate;
		TranslationHostPrivate::Instance = this;
		YSSCore::General::YSSCoreTranslator* coreTranslator = new YSSCore::General::YSSCoreTranslator();
		active(coreTranslator);
	}
	TranslationHost::~TranslationHost() {
		delete d;
	}
	TranslationHost* TranslationHost::getInstance() {
		return TranslationHostPrivate::Instance;
	}
	void TranslationHost::setLangID(Translator::LangID id) {
		if (id != d->GlobalID) {
			d->refreshLang(id);
		}
		d->GlobalID = id;
	}
	void TranslationHost::active(Translator* translator) {
		d->Translators.insert(translator->getNamespace(), translator);
		translator->d->loadDefault();
		translator->d->loadTranslationFile(d->GlobalID);
	}
	QString TranslationHost::tr(const QString& nameSpace, const QString& key) {
		Translator* trans = d->Translators[nameSpace];
		if (trans != nullptr)[[likely]] {
			return trans->tr(key);
		}
		else {
			return nameSpace + "::" + key;
		}
	}
}