#include "../ThemeManager.h"
#include <QtGui>
namespace YSSCore::Editor {
	ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
		
	}
	void ThemeManager::loadConfig(const QString& path) {
		QFile file(path);
		if (file.open(QIODevice::ReadOnly)) {
			if (Config != nullptr) {
				delete Config;
			}
			Config = new YSSCore::Utility::JsonConfig(file.readAll());
			file.close();
		}
		else {
			qDebug() << "ThemeManager: loadConfig failed!";
		}
	}
	QColor ThemeManager::getColor(const QString& key) {
		QString color = Config->getString(key);
		if (color.startsWith("#")) {
			return QColor(color);
		}
		else {
			return QColor("#ED1C24");
		}
	}
	QString ThemeManager::getColorString(const QString& key) {
		QString color = Config->getString(key);
		if (color.startsWith("#")) {
			return color;
		}
		else {
			return "#ED1C24";
		}
	}
}