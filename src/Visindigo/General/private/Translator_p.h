#ifndef Visindigo_General_private_Translator_p_h
#define Visindigo_General_private_Translator_p_h
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include "General/Translator.h"
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
		QString ID;
		QMap<QString, QString> DefaultStrings;
		QMap<QString, QString> LangStrings;
		Visindigo::General::Translator::LangID DefaultID = Visindigo::General::Translator::LangID::zh_CN;
		QMap<Visindigo::General::Translator::LangID, QString> FilePath = {};
		~TranslatorPrivate();
		bool loadDefault();
		bool loadLanguage(Visindigo::General::Translator::LangID id);
		bool loadStrings(Visindigo::General::Translator::LangID id, QMap<QString, QString>& target);
	};
}
#endif // Visindigo_General_private_Translator_p_h
