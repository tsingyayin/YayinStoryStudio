#pragma once
#include <QtCore/qobject.h>
namespace Visindigo {
	namespace Utility {
		class JsonConfig;
		class PathMacro;
	};
	namespace Editor {
		class LangServerManager;
		class EditorPluginManager;
		class FileServerManager;
		class ProjectTemplateManager;
		class FileTemplateManager;
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
		Visindigo::Utility::JsonConfig* Config = nullptr;
		Visindigo::Utility::PathMacro* PathMacro = nullptr;
		Visindigo::Widgets::ThemeManager* Theme = nullptr;
		Visindigo::Editor::LangServerManager* LangServerManager = nullptr;
		Visindigo::Editor::EditorPluginManager* PluginManager = nullptr;
		Visindigo::Editor::FileServerManager* FileServerManager = nullptr;
		Visindigo::Editor::ProjectTemplateManager* ProjectTemplateManager = nullptr;
		Visindigo::Editor::FileTemplateManager* FileTemplateManager = nullptr;
		YSS::Editor::YSSTranslator* YSSTranslator = nullptr;
		Visindigo::General::TranslationHost* TranslationHost = nullptr;
		YSS::Editor::MainWin* MainWindow = nullptr;
	public:
		GlobalValue();
		static GlobalValue* getInstance();
		static QColor getColor(const QString& key);
		static Visindigo::Utility::JsonConfig* getConfig();
		static void saveConfig();
		static Visindigo::Editor::LangServerManager* getLangServerManager();
		static void setMainWindow(YSS::Editor::MainWin* mainWindow);
		static YSS::Editor::MainWin* getMainWindow();
		static void setCurrentProject(Visindigo::General::YSSProject* project);
		static Visindigo::General::YSSProject* getCurrentProject();
		void loadConfig();
	};
}