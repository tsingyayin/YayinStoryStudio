#pragma once
#include <QtCore>
#include "../Utility/JsonConfig.h"
#include "ThemeManager.h"
namespace YSS {
	class GlobalValue :public QObject
	{
		Q_OBJECT;
	private:
		static GlobalValue* Instance;
		Utility::JsonConfig* Config = nullptr;
		Utility::JsonConfig* Language = nullptr;
		ThemeManager* Theme = nullptr;
	public:
		GlobalValue();
		static const GlobalValue* getInstance();
		static QColor getColor(const QString& key);
		static QString getTr(const QString& key);
		static const Utility::JsonConfig* getConfig();
		void loadConfig();
		void loadLanguage();
	};
}