#pragma once
#include <QtCore/qobject.h>
namespace YSSCore {
	namespace Utility {
		class JsonConfig;
		class PathMacro;
	};
	namespace Editor {
		class LangServerManager;
		class EditorPluginManager;
		class FileServerManager;
		class ProjectTemplateManager;
	};
	namespace General {
		class TranslationHost;
		class YSSProject;
	};
	namespace Widgets {
		class ThemeManager;
	}
};
namespace YSS {
	namespace Editor {
		class MainWin;
		class YSSTranslator;
	}
	class GlobalValue :public QObject
	{
		Q_OBJECT;
	private:
		static GlobalValue* Instance;
		YSSCore::Utility::JsonConfig* Config = nullptr;
		YSSCore::Utility::PathMacro* PathMacro = nullptr;
		YSSCore::Widgets::ThemeManager* Theme = nullptr;
		YSSCore::Editor::LangServerManager* LangServerManager = nullptr;
		YSSCore::Editor::EditorPluginManager* PluginManager = nullptr;
		YSSCore::Editor::FileServerManager* FileServerManager = nullptr;
		YSSCore::Editor::ProjectTemplateManager* TemplateManager = nullptr;
		YSS::Editor::YSSTranslator* YSSTranslator = nullptr;
		YSSCore::General::TranslationHost* TranslationHost = nullptr;
		YSS::Editor::MainWin* MainWindow = nullptr;
	public:
		GlobalValue();
		static GlobalValue* getInstance();
		static QColor getColor(const QString& key);
		static YSSCore::Utility::JsonConfig* getConfig();
		static void saveConfig();
		static YSSCore::Editor::LangServerManager* getLangServerManager();
		static void setMainWindow(YSS::Editor::MainWin* mainWindow);
		static YSS::Editor::MainWin* getMainWindow();
		static void setCurrentProject(YSSCore::General::YSSProject* project);
		static YSSCore::General::YSSProject* getCurrentProject();
		void loadConfig();
	};
}