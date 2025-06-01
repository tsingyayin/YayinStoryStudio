#include "../GlobalValue.h"
#include "../YSSTranslator.h"
#include <Utility/JsonConfig.h>
#include <Utility/PathMacro.h>
#include <Editor/ThemeManager.h>
#include <Editor/LangServerManager.h>
#include <Editor/EditorPluginManager.h>
#include <Editor/FileServerManager.h>
#include <General/TranslationHost.h>
#include <QtCore/qfile.h>
#include <QtGui/qcolor.h>

namespace YSS {
	GlobalValue* GlobalValue::Instance = nullptr;
	GlobalValue::GlobalValue() {
		if (Instance != nullptr) {
			throw std::runtime_error("GlobalValue already exists!");
		}
		Instance = this;
		PathMacro = new YSSCore::Utility::PathMacro();
		Theme = new YSSCore::Editor::ThemeManager(this);
		loadConfig();
		LangServerManager = new YSSCore::Editor::LangServerManager();
		FileServerManager = new YSSCore::Editor::FileServerManager();
		TranslationHost = new YSSCore::General::TranslationHost();
		YSSTranslator = new YSS::Editor::YSSTranslator();
		TranslationHost->active(YSSTranslator);
		PluginManager = new YSSCore::Editor::EditorPluginManager();
		PluginManager->programLoadPlugin();
		PluginManager->loadPlugin();
	}
	QColor GlobalValue::getColor(const QString& key) {
		return Instance->Theme->getColor(key);
	}

	YSSCore::Utility::JsonConfig* GlobalValue::getConfig() {
		return Instance->Config;
	}
	void GlobalValue::saveConfig() {
		if (Instance->Config != nullptr) {
			QFile file("./resource/editor_config.json");
			if (file.open(QIODevice::WriteOnly)) {
				file.write(Instance->Config->toString().toUtf8());
				file.close();
			}
			else {
				qDebug() << "MainWin: saveConfig failed!";
			}
		}
	}
	GlobalValue* GlobalValue::getInstance() {
		return Instance;
	}

	void GlobalValue::loadConfig() {
		if (Config != nullptr) {
			delete Config;
		}
		Config = new YSSCore::Utility::JsonConfig();
		QFile file("./resource/editor_config.json");
		QTextStream in(&file);
		in.setEncoding(QStringConverter::Utf8);
		if (file.open(QIODevice::ReadOnly)) {
			Config->parse(in.readAll());
			file.close();
			qDebug() << "MainWin: loadConfig success!";
		}
		else {
			qDebug() << "MainWin: loadConfig failed!";
		}
		Theme->loadConfig("./resource/theme/config/" + Config->getString("Preference.Theme") + ".json");
	}
	void GlobalValue::setMainWindow(YSS::Editor::MainWin* mainWindow) {
		Instance->MainWindow = mainWindow;
	}
	YSS::Editor::MainWin* GlobalValue::getMainWindow() {
		return Instance->MainWindow;
	}

	YSSCore::Editor::LangServerManager* GlobalValue::getLangServerManager() {
		return Instance->LangServerManager;
	}
	
	void GlobalValue::setCurrentProject(YSSCore::General::YSSProject* project) {
		Instance->CurrentProject = project;
	}
}