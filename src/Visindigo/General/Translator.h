#ifndef Visindigo_General_Translator_h
#define Visindigo_General_Translator_h
#include <QtCore/qmap.h>
#include "General/PluginModule.h"
#include "VICompileMacro.h"
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
	class VisindigoAPI Translator :public PluginModule {
		friend class Visindigo::__Private__::TranslatorPrivate;
		friend class TranslationHostPrivate;
		friend class TranslationHost;
		Q_OBJECT;
	public:
		enum LangID {
			Unknown = 0,
			zh_CN,
			zh_SC = zh_CN,
			zh_TC,
			en,
			ja,
			ko,
			ru,
			de,
			fr,
		};
		Q_ENUM(LangID);
	public:
		static QString langIDToString(LangID id);
		static LangID stringToLangID(const QString& str);
	public:
		Translator(Plugin* parent, const QString& nameSpace);
		virtual ~Translator();
		void setNamespace(const QString& nameSpace);
		QString getNamespace();
		void setDefaultLang(LangID lang);
		void setLangFilePath(const QMap<LangID, QString>& langFilePath);
		void addLangFilePath(LangID id, QString filePath);
		QString tr(const QString& key);
	protected:
		Visindigo::__Private__::TranslatorPrivate* d;
	};
}

#endif // Visindigo_General_Translator_h