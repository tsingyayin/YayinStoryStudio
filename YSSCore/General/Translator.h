#pragma once
#include <QtCore/qmap.h>
#include "../Macro.h"
class QString;
namespace YSSCore::__Private__ {
	class TranslatorPrivate;
}
namespace YSSCore::General {
	class TranslationHostPrivate;
	class TranslationHost;
	class YSSCoreAPI Translator {
		friend class YSSCore::__Private__::TranslatorPrivate;
		friend class TranslationHostPrivate;
		friend class TranslationHost;
	public:
		enum LangID {
			Unknown = 0,
			zh_CN,
			zh_SC = zh_CN,
			zh_TC,
			en_US,
			en_GB,
			ja_JP,
			ko_KR,
			ru_RU,
			de_DE,
			fr_FR,
		};
	public:
		Translator();
		Translator(const QString& nameSpace);
		void setNamespace(const QString& nameSpace);
		QString getNamespace();
		void setDefaultLang(LangID lang);
		void setLangFilePath(QMap<LangID, QString> langFilePath);
		QString tr(const QString& key);
	protected:
		YSSCore::__Private__::TranslatorPrivate* d;
	};
}