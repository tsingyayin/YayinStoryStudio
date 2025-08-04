#include "../private/Translator_p.h"
#include "../../Utility/JsonDocument.h"
#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

namespace Visindigo::__Private__ {
	Visindigo::General::VisindigoTranslator* TranslatorPrivate::Instance = nullptr;

	bool TranslatorPrivate::loadDefault() {
		return loadTranslationFile(DefaultID, true);
	}
	bool TranslatorPrivate::loadTranslationFile(Visindigo::General::Translator::LangID id, bool asDefault) {
		QString filePath = FilePath[id];
		QFile file(filePath);
		if (!file.exists()) {
			return false;
		}
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		QString jsonConfig = ts.readAll();
		file.close();
		if (asDefault) {
			if (DefaultLang != nullptr) {
				delete DefaultLang;
			}
			DefaultLang = new Visindigo::Utility::JsonConfig();
			DefaultLang->parse(jsonConfig);
		}
		else {
			if (Lang != nullptr) {
				delete Lang;
			}
			Lang = new Visindigo::Utility::JsonConfig();
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
namespace Visindigo::General
{
	Translator::Translator() {
		d = new Visindigo::__Private__::TranslatorPrivate;
	}
	Translator::Translator(const QString& nameSpace) {
		d = new Visindigo::__Private__::TranslatorPrivate;
		setNamespace(nameSpace);
	}
	void Translator::setNamespace(const QString& nameSpace) {
		d->ID = nameSpace;
	}
	QString Translator::getNamespace() {
		return d->ID;
	}
	void Translator::setDefaultLang(LangID lang) {
		d->DefaultID = lang;
	}
	void Translator::setLangFilePath(QMap<LangID, QString> langFilePath) {
		d->FilePath = langFilePath;
	}
	void Translator::addLangFilePath(LangID id, QString path) {
		d->FilePath.insert(id, path);
	}
	QString Translator::tr(const QString& key) {
		if (d->Lang == nullptr && d->DefaultLang == nullptr) [[unlikely]] {
			return key;
		}
		if (d->Lang != nullptr) [[likely]] {
			QString tr = d->Lang->getString(key);
			if (tr.isEmpty()) [[unlikely]] {
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
			else [[likely]] {
				return tr;
			}
		}
		else {
			if (d->DefaultLang != nullptr) {
				QString tr = d->DefaultLang->getString(key);
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
	}

	VisindigoTranslator::VisindigoTranslator() {
		Visindigo::__Private__::TranslatorPrivate::Instance = this;
		setNamespace("Visindigo");
		setDefaultLang(zh_CN);
		setLangFilePath(
			{
				{zh_CN, ":/Visindigo/compiled/i18n/zh_CN.json"},
				{en_US, ":/Visindigo/compiled/i18n/en_US.json"}
			}
		);
	}

	VisindigoTranslator* VisindigoTranslator::getInstance() {
		return Visindigo::__Private__::TranslatorPrivate::Instance;
	}
}