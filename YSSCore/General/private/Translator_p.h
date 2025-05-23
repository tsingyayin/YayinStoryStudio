#pragma once
#include "../Translator.h"
#include "../../Utility/JsonConfig.h"
namespace YSSCore::General {
	class TranslationHost;
	class TranslationHostPrivate;
	class YSSCoreTranslator;
};
namespace YSSCore::__Private__ {
	class TranslatorPrivate {
		friend class YSSCore::General::Translator;
		friend class YSSCore::General::TranslationHost;
		friend class YSSCore::General::TranslationHostPrivate;
		friend class YSSCore::General::YSSCoreTranslator;
	protected:
		static YSSCore::General::YSSCoreTranslator* Instance;
		QString ID;
		QMap<YSSCore::General::Translator::LangID, QString> FilePath;
		YSSCore::Utility::JsonConfig* DefaultLang = nullptr;
		YSSCore::Utility::JsonConfig* Lang = nullptr;
		YSSCore::General::Translator::LangID DefaultID = YSSCore::General::Translator::LangID::zh_CN;
		~TranslatorPrivate();
		bool loadDefault();
		bool loadTranslationFile(YSSCore::General::Translator::LangID id, bool asDefault = false);
	};
}