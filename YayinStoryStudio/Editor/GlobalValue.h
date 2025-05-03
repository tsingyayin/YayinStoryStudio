#pragma once
#include <QtCore>
#include <Utility/JsonConfig.h>
#include <Editor/ThemeManager.h>
#include <Editor/LangServerManager.h>
namespace YSS {
	class GlobalValue :public QObject
	{
		Q_OBJECT;
	private:
		static GlobalValue* Instance;
		YSSCore::Utility::JsonConfig * Config = nullptr;
		YSSCore::Utility::JsonConfig* Language = nullptr;
		YSSCore::Editor::ThemeManager* Theme = nullptr;
		YSSCore::Editor::LangServerManager* LangServerManager = nullptr;
	public:
		GlobalValue();
		static const GlobalValue* getInstance();
		static QColor getColor(const QString& key);
		static QString getTr(const QString& key);
		static const YSSCore::Utility::JsonConfig* getConfig();
		static YSSCore::Editor::LangServerManager* getLangServerManager();
		void loadConfig();
		void loadLanguage();
	};
}