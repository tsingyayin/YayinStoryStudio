#ifndef Visindigo_General_TranslationHost_h
#define Visindigo_General_TranslationHost_h
#include <QtCore/qobject.h>
#include "VICompileMacro.h"
#include "General/Translator.h"
// Forward declarations
class QString;
namespace Visindigo::General {
	class Translator;
	class TranslationHostPrivate;
}
using VI18NStr = QString;
// Main
namespace Visindigo::General {
	class VisindigoAPI TranslationHost :public QObject
	{
		Q_OBJECT;
	signals:
		void langChanged(Translator::LangID id);
	private:
		TranslationHost();
	public:
		static TranslationHost* getInstance();	
		~TranslationHost();
		void setLangID(Translator::LangID id);
		void registerTranslator(Translator* translator);
		void unregisterTranslator(Translator* translator);
		QString tra(const QString& nameSpace, const QString& key);
		QString tra(const QString& key);
		QString i18n(const VI18NStr& raw);
	private:
		TranslationHostPrivate* d;
	};
}
// Helper macros
#define VITRH Visindigo::General::TranslationHost::getInstance()
#define VITR Visindigo::General::TranslationHost::getInstance()->tra
#define VI18N Visindigo::General::TranslationHost::getInstance()->i18n

#endif // Visindigo_General_TranslationHost_h