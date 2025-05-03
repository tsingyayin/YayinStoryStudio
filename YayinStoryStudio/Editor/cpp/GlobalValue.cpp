#include "../GlobalValue.h"
#include <QtGui>
namespace YSS {
	GlobalValue* GlobalValue::Instance = nullptr;
	GlobalValue::GlobalValue() {
		if (Instance != nullptr) {
			throw std::runtime_error("GlobalValue already exists!");
		}
		Instance = this;
		Theme = new YSSCore::Editor::ThemeManager(this);
		LangServerManager = new YSSCore::Editor::LangServerManager();
		loadConfig();
		loadLanguage();
	}
	QColor GlobalValue::getColor(const QString& key) {
		return Instance->Theme->getColor(key);
	}

	QString GlobalValue::getTr(const QString& key) {
		return Instance->Language->getString(key);
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
		Theme->loadConfig("./resource/theme/" + Config->getString("Preferences.Theme") + ".json");
	}
	void GlobalValue::setMainWindow(YSS::Editor::MainWin* mainWindow) {
		Instance->MainWindow = mainWindow;
	}
	YSS::Editor::MainWin* GlobalValue::getMainWindow() {
		return Instance->MainWindow;
	}
	void GlobalValue::loadLanguage() {
		if (Language != nullptr) {
			delete Language;
		}
		Language = new YSSCore::Utility::JsonConfig();
		QFile file("./resource/lang/" + Config->getString("Preferences.Language") + ".json");
		QTextStream in(&file);
		in.setEncoding(QStringConverter::Utf8);
		if (file.open(QIODevice::ReadOnly)) {
			Language->parse(in.readAll());
			file.close();
		}
		else {
			qDebug() << "GlobalValue: loadLanguage failed!";
		}
		qDebug() << GlobalValue::getTr("General.File");
	}

	YSSCore::Editor::LangServerManager* GlobalValue::getLangServerManager() {
		return Instance->LangServerManager;
	}
}