#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"
class QString;
namespace YSSCore::General {
	class TranslatorPrivate;
	class YSSCoreAPI Translator :public QObject {
		Q_OBJECT;
		friend class TranslatorPrivate;
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
	signals:
		void languageChanged();
	public:
		Translator();
		~Translator();
		static Translator* globalInstance();
		bool loadTranslation(const QString& langPath, const QString& defaultPath);
		QString tr(const QString& key);
	private:
		TranslatorPrivate* d;
	};
}