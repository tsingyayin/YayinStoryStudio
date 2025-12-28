#include "../GlobalValue.h"
#include "../YSSTranslator.h"
#include <Utility/JsonConfig.h>
#include <Utility/PathMacro.h>
#include <Widgets/ThemeManager.h>
#include <QtCore/qtextstream.h>
#include <General/TranslationHost.h>
#include <QtCore/qfile.h>
#include <QtGui/qcolor.h>
#include <General/Log.h>
#include <General/YSSProject.h>
#include <General/YSSLogger.h>
namespace YSS {
	GlobalValue* GlobalValue::Instance = nullptr;
	GlobalValue::GlobalValue() {
		Instance = this;
		yMessageF << "Program global value initializing";
		PathMacro = new Visindigo::Utility::PathMacro();
		Theme = new Visindigo::Widgets::ThemeManager(this);
		loadConfig();
		YSSTranslator = new YSS::Editor::YSSTranslator();
		Visindigo::General::TranslationHost::getInstance()->active(YSSTranslator);
		ySuccessF << "Program global value initialized !";
		//PluginManager = new YSSCore::Editor::EditorPluginManager();
		//PluginManager->loadAllPlugin();
		//PluginManager->enableAllPlugin();
	}
	GlobalValue::~GlobalValue() {
		// Nothing to do
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
				vgErrorF << "failed";
			}
		}
	}
	GlobalValue* GlobalValue::getInstance() {
		if (Instance == nullptr) {
			Instance = new GlobalValue();
		}
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
			vgErrorF << "failed";
		}
		Theme->loadConfig("./resource/theme/config/" + Config->getString("Preference.Theme") + ".json");
	}
	void GlobalValue::setMainWindow(YSS::Editor::MainWin* mainWindow) {
		Instance->MainWindow = mainWindow;
	}
	YSS::Editor::MainWin* GlobalValue::getMainWindow() {
		return Instance->MainWindow;
	}

	void GlobalValue::setCurrentProject(YSSCore::General::YSSProject* project) {
		YSSCore::General::YSSProject::setCurrentProject(project);
	}

	YSSCore::General::YSSProject* GlobalValue::getCurrentProject() {
		return YSSCore::General::YSSProject::getCurrentProject();
	}
}