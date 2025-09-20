#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"
#include "Translator.h"
// Forward declarations
class QString;
namespace Visindigo::General {
	class Translator;
	class TranslationHostPrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI TranslationHost :public QObject
	{
		Q_OBJECT;
	signals:
		void langChanged(Translator::LangID id);
	public:
		static TranslationHost* getInstance();
		TranslationHost();
		~TranslationHost();
		void setLangID(Translator::LangID id);
		void active(Translator* translator);
		QString tr(const QString& nameSpace, const QString& key);
		QString tr(const QString& key);
		QString i18n(const QString& raw);
	private:
		TranslationHostPrivate* d;
	};
}
// Helper macros
#define YSSTR Visindigo::General::TranslationHost::getInstance()->tr
#define YSSI18N Visindigo::General::TranslationHost::getInstance()->i18n