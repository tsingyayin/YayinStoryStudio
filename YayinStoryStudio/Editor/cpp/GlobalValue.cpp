#include "../GlobalValue.h"
#include <QtGui>
namespace YSS {
	GlobalValue* GlobalValue::Instance = nullptr;
	GlobalValue::GlobalValue() {
		Instance = this;
		Theme = new ThemeManager(this);
		loadConfig();
		loadLanguage();
	}
	QColor GlobalValue::getColor(const QString& key) {
		return Instance->Theme->getColor(key);
	}

	QString GlobalValue::getTr(const QString& key) {
		return Instance->Language->getString(key);
	}
	const Utility::JsonConfig* GlobalValue::getConfig() {
		return Instance->Config;
	}

	const GlobalValue* GlobalValue::getInstance() {
		return Instance;
	}

	void GlobalValue::loadConfig() {
		if (Config != nullptr) {
			delete Config;
		}
		Config = new Utility::JsonConfig();
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

	void GlobalValue::loadLanguage() {
		if (Language != nullptr) {
			delete Language;
		}
		Language = new Utility::JsonConfig();
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
}