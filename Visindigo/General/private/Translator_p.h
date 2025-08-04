#pragma once
#include "../Translator.h"
#include "../../Utility/JsonConfig.h"
namespace Visindigo::General {
	class TranslationHost;
	class TranslationHostPrivate;
	class VisindigoTranslator;
};
namespace Visindigo::__Private__ {
	class TranslatorPrivate {
		friend class Visindigo::General::Translator;
		friend class Visindigo::General::TranslationHost;
		friend class Visindigo::General::TranslationHostPrivate;
		friend class Visindigo::General::VisindigoTranslator;
	protected:
		static Visindigo::General::VisindigoTranslator* Instance;
		QString ID;
		QMap<Visindigo::General::Translator::LangID, QString> FilePath;
		Visindigo::Utility::JsonConfig* DefaultLang = nullptr;
		Visindigo::Utility::JsonConfig* Lang = nullptr;
		Visindigo::General::Translator::LangID DefaultID = Visindigo::General::Translator::LangID::zh_CN;
		~TranslatorPrivate();
		bool loadDefault();
		bool loadTranslationFile(Visindigo::General::Translator::LangID id, bool asDefault = false);
	};
}