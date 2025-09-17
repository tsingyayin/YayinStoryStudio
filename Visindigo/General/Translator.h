#pragma once
#include <QtCore/qmap.h>
#include "../Macro.h"
// Forward declarations
class QString;
namespace Visindigo::__Private__ {
	class TranslatorPrivate;
}
namespace Visindigo::General {
	class TranslationHostPrivate;
	class TranslationHost;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI Translator {
		friend class Visindigo::__Private__::TranslatorPrivate;
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
		void addLangFilePath(LangID id, QString filePath);
		QString tr(const QString& key);
	protected:
		Visindigo::__Private__::TranslatorPrivate* d;
	};

	class VisindigoAPI VisindigoTranslator : public Translator {
	public:
		VisindigoTranslator();
		static VisindigoTranslator* getInstance();
	};
}