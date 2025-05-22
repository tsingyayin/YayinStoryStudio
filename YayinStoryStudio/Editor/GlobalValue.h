#pragma once
#include <QtCore/qobject.h>
namespace YSSCore {
	namespace Utility {
		class JsonConfig;
	};
	namespace Editor {
		class ThemeManager;
		class LangServerManager;
		class EditorPluginManager;
		class FileServerManager;
	};
	namespace General {
		class TranslationHost;
	}
};
namespace YSS {
	namespace Editor {
		class MainWin;
	}
	class GlobalValue :public QObject
	{
		Q_OBJECT;
	private:
		static GlobalValue* Instance;
		YSSCore::Utility::JsonConfig* Config = nullptr;
		YSSCore::Utility::JsonConfig* Language = nullptr;
		YSSCore::Editor::ThemeManager* Theme = nullptr;
		YSSCore::Editor::LangServerManager* LangServerManager = nullptr;
		YSSCore::Editor::EditorPluginManager* PluginManager = nullptr;
		YSSCore::Editor::FileServerManager* FileServerManager = nullptr;
		YSSCore::General::TranslationHost* TranslationHost = nullptr;
		YSS::Editor::MainWin* MainWindow = nullptr;
	public:
		GlobalValue();
		static GlobalValue* getInstance();
		static QColor getColor(const QString& key);
		static QString getTr(const QString& key);
		static YSSCore::Utility::JsonConfig* getConfig();
		static void saveConfig();
		static YSSCore::Editor::LangServerManager* getLangServerManager();
		static void setMainWindow(YSS::Editor::MainWin* mainWindow);
		static YSS::Editor::MainWin* getMainWindow();
		void loadConfig();
		void loadLanguage();
	};
}