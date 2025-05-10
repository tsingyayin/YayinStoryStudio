#include "../Translatior.h"
#include "../../Utility/JsonDocument.h"
#include <QtCore/qstring.h>

namespace YSSCore::General
{
	class TranslatorPrivate {
		friend class Translator;
	protected:
		Utility::JsonDocument JsonDoc;
		static Translator* GlobalInstance;
	};

	Translator* TranslatorPrivate::GlobalInstance = nullptr;

	Translator::Translator() {
		if (TranslatorPrivate::GlobalInstance == nullptr) {
			TranslatorPrivate::GlobalInstance = this;
		}
		d = new TranslatorPrivate();
	}

	Translator::~Translator() {
		delete d;
	}

	Translator* Translator::globalInstance() {
		return TranslatorPrivate::GlobalInstance;
	}

	bool Translator::loadTranslation(const QString& langPath, const QString& defaultPath) {
		if (langPath.isEmpty()) {
			return false;
		}
		uchar error = 0;
		if (d->JsonDoc.load(langPath, defaultPath,&error,false).error != QJsonParseError::NoError) {
			return false;
		}
		return true;
	}

	QString Translator::tr(const QString& key) {
		if (key.isEmpty()) {
			return "";
		}
		if (d->JsonDoc.contains(key)) {
			return d->JsonDoc.getString(key);
		}
		return key;
	}
}