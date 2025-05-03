#include "AStoryTheme.h"
#include "AStory.h"
#include <Editor/ThemeManager.h>
#include <Utility/JsonConfig.h>

AStoryTheme* AStoryTheme::Instance = nullptr;

AStoryTheme::AStoryTheme(AStoryLanguageServer* server){
	Instance = this;
	this->Server = server;
	loadTheme();
}
AStoryTheme* AStoryTheme::getInstance() {
	return Instance;
}
void AStoryTheme::loadTheme() {
	QString path = Server->getPluginFolder();
	QString ID = YSSCore::Editor::ThemeManager::getInstance()->getThemeID();
	YSSCore::Utility::JsonConfig config;
	QFile file(path + "/meta.json");
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		in.setEncoding(QStringConverter::Utf8);
		config.parse(in.readAll());
		file.close();
	}
	else {
		qDebug() << "AStoryTheme: load theme failed!";
		return;
	}
	QString themePath = config.getString("Theme." + ID);
	if (themePath.isEmpty()) {
		qDebug() << "AStoryTheme: theme path is empty!";
		return;
	}
	file.setFileName(path + "/" + themePath);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		in.setEncoding(QStringConverter::Utf8);
		if (Config != nullptr) {
			delete Config;
		}
		Config = new YSSCore::Utility::JsonConfig();
		Config->parse(in.readAll());
		file.close();
	}
	else {
		qDebug() << "AStoryTheme: load theme config failed!";
		return;
	}
}

QColor AStoryTheme::getColor(const QString& key) {
	if (Config == nullptr) {
		return QColor();
	}
	if (Config->contains(key)) {
		return Config->getValue(key).toVariant().value<QColor>();
	}
	else {
		qDebug() << "AStoryTheme: key not found!";
		return QColor();
	}
}

QString AStoryTheme::getColorString(const QString& key) {
	if (Config == nullptr) {
		return "";
	}
	if (Config->contains(key)) {
		return Config->getValue(key).toString();
	}
	else {
		qDebug() << "AStoryTheme: key not found!";
		return "";
	}
}