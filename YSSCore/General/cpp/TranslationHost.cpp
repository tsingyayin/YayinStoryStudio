#include "../TranslationHost.h"
#include "../private/Translator_p.h"
#include "../Log.h"
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
		ySuccessF << "Success!";
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
		yNotice << "Translator:" << translator->getNamespace() << "actived.";
	}
	QString TranslationHost::tr(const QString& nameSpace, const QString& key) {
		Translator* trans = d->Translators[nameSpace];
		if (trans != nullptr) [[likely]] {
			return trans->tr(key);
		}
		else {
			return nameSpace + "::" + key;
		}
	}
	QString TranslationHost::tr(const QString& key) {
		QStringList keys = key.split("::");
		if (keys.length() == 2) {
			return tr(keys[0], keys[1]);
		}
		return key;
	}
	QString TranslationHost::i18n(const QString& raw) {
		if (raw.startsWith("i18n:")) {
			QStringList keys = raw.mid(5).split("::");
			if (keys.length() != 2) {
				return raw;
			}
			return tr(keys[0], keys[1]);
		}
		return raw;
	}
}