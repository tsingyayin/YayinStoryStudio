#include "../private/Translator_p.h"
#include "../../Utility/JsonDocument.h"
#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

namespace YSSCore::__Private__ {
	bool TranslatorPrivate::loadDefault() {
		return loadTranslationFile(DefaultID, true);
	}
	bool TranslatorPrivate::loadTranslationFile(YSSCore::General::Translator::LangID id, bool asDefault) {
		QString filePath = FilePath[id];
		QFile file(filePath);
		if (!file.exists()) {
			return false;
		}
		file.open(QIODevice::ReadOnly|QIODevice::Text);
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		QString jsonConfig = ts.readAll();
		file.close();
		if (asDefault) {
			if (DefaultLang != nullptr) {
				delete DefaultLang;
			}
			DefaultLang = new YSSCore::Utility::JsonConfig();
			DefaultLang->parse(jsonConfig);
		}
		else {
			if (Lang != nullptr) {
				delete Lang;
			}
			Lang = new YSSCore::Utility::JsonConfig();
			Lang->parse(jsonConfig);
		}
		return true;
	}
	TranslatorPrivate::~TranslatorPrivate() {
		if (DefaultLang != nullptr) {
			delete DefaultLang;
		}
		if (Lang != nullptr) {
			delete Lang;
		}
	}
}
namespace YSSCore::General
{
	Translator::Translator() {
		d = new YSSCore::__Private__::TranslatorPrivate;
	}
	Translator::Translator(const QString& nameSpace) {
		d = new YSSCore::__Private__::TranslatorPrivate;
		setNamespace(nameSpace);
	}
	void Translator::setNamespace(const QString& nameSpace) {
		d->ID = nameSpace;
	}
	QString Translator::getNamespace() {
		return d->ID;
	}
	void Translator::setDefaultLang(LangID lang) {
		d->DefaultID= lang;
	}
	void Translator::setLangFilePath(QMap<LangID, QString> langFilePath) {
		d->FilePath = langFilePath;
	}
	QString Translator::tr(const QString& key) {
		if (d->Lang == nullptr && d->DefaultLang == nullptr)[[unlikely]] {
			return key;
		}
		if (d->Lang != nullptr) [[likely]]{
			QString tr = d->Lang->getString(key);
			if (tr.isEmpty())[[unlikely]] {
				if (d->DefaultLang != nullptr) {
					tr = d->DefaultLang->getString(key);
					if (tr.isEmpty()) {
						return key;
					}
					else {
						return tr;
					}
				}
				else {
					return key;
				}
			}
			else[[likely]] {
				return key;
			}
		}
	}
}