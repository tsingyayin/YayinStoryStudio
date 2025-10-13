#include "../GlobalValue.h"
#include "../YSSTranslator.h"
#include <Utility/JsonConfig.h>
#include <Utility/PathMacro.h>
#include <Widgets/ThemeManager.h>
#include <Editor/LangServerManager.h>
#include <Editor/EditorPluginManager.h>
#include <Editor/FileServerManager.h>
#include <Editor/ProjectTemplateProvider.h>
#include <Editor/ProjectTemplateManager.h>
#include <Editor/FileTemplateManager.h>
#include <General/TranslationHost.h>
#include <QtCore/qfile.h>
#include <QtGui/qcolor.h>
#include <General/Log.h>
#include <General/YSSProject.h>

namespace YSS {
	GlobalValue* GlobalValue::Instance = nullptr;
	GlobalValue::GlobalValue() {
		if (Instance != nullptr) {
			throw std::runtime_error("GlobalValue already exists!");
		}
		Instance = this;
		yMessage << "Program global value initializing";
		PathMacro = new Visindigo::Utility::PathMacro();
		Theme = new Visindigo::Widgets::ThemeManager(this);
		loadConfig();
		TranslationHost = new Visindigo::General::TranslationHost();
		YSSTranslator = new YSS::Editor::YSSTranslator();
		TranslationHost->active(YSSTranslator);
		LangServerManager = new Visindigo::Editor::LangServerManager();
		FileServerManager = new Visindigo::Editor::FileServerManager();
		ProjectTemplateManager = new Visindigo::Editor::ProjectTemplateManager();
		FileTemplateManager = new Visindigo::Editor::FileTemplateManager();
		ySuccess << "Program global value initialized !";
		PluginManager = new Visindigo::Editor::EditorPluginManager();
		PluginManager->loadAllPlugin();
		PluginManager->enableAllPlugin();
	}
	QColor GlobalValue::getColor(const QString& key) {
		return Instance->Theme->getColor(key);
	}

	Visindigo::Utility::JsonConfig* GlobalValue::getConfig() {
		return Instance->Config;
	}
	void GlobalValue::saveConfig() {
		if (Instance->Config != nullptr) {
			QFile file("./resource/editor_config.json");
			if (file.open(QIODevice::WriteOnly)) {
				file.write(Instance->Config->toString().toUtf8());
				file.close();
				ySuccessF << "saved";
			}
			else {
				yErrorF << "failed";
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
		Config = new Visindigo::Utility::JsonConfig();
		QFile file("./resource/editor_config.json");
		QTextStream in(&file);
		in.setEncoding(QStringConverter::Utf8);
		if (file.open(QIODevice::ReadOnly)) {
			Config->parse(in.readAll());
			file.close();
			ySuccessF << "success";
		}
		else {
			yErrorF << "failed";
		}
		Theme->loadConfig("./resource/theme/config/" + Config->getString("Preference.Theme") + ".json");
	}
	void GlobalValue::setMainWindow(YSS::Editor::MainWin* mainWindow) {
		Instance->MainWindow = mainWindow;
	}
	YSS::Editor::MainWin* GlobalValue::getMainWindow() {
		return Instance->MainWindow;
	}

	Visindigo::Editor::LangServerManager* GlobalValue::getLangServerManager() {
		return Instance->LangServerManager;
	}

	void GlobalValue::setCurrentProject(Visindigo::General::YSSProject* project) {
		Visindigo::General::YSSProject::setCurrentProject(project);
	}

	Visindigo::General::YSSProject* GlobalValue::getCurrentProject() {
		return Visindigo::General::YSSProject::getCurrentProject();
	}
}