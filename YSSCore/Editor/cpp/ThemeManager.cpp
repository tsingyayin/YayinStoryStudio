#include "../ThemeManager.h"
#include <QtGui>
namespace YSSCore::Editor {
	class ThemeManagerPrivate {
		friend class ThemeManager;
	protected:
		YSSCore::Utility::JsonConfig* Config = nullptr;
		static ThemeManager* Instance;
	};
	ThemeManager* ThemeManagerPrivate::Instance = nullptr;
	ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
		p = new ThemeManagerPrivate();
		p->Instance = this;
	}
	ThemeManager* ThemeManager::getInstance() {
		return ThemeManagerPrivate::Instance;
	}
	void ThemeManager::loadConfig(const QString& path) {
		QFile file(path);
		if (file.open(QIODevice::ReadOnly)) {
			if (p->Config != nullptr) {
				delete p->Config;
			}
			p->Config = new YSSCore::Utility::JsonConfig(file.readAll());
			file.close();
		}
		else {
			qDebug() << "ThemeManager: loadConfig failed!";
		}
	}
	QString ThemeManager::getThemeName() {
		return p->Config->getString("ThemeName");
	}
	QString ThemeManager::getThemeID() {
		return p->Config->getString("ThemeID");
	}
	QColor ThemeManager::getColor(const QString& key) {
		QString color = p->Config->getString(key);
		if (color.startsWith("#")) {
			return QColor(color);
		}
		else {
			return QColor("#ED1C24");
		}
	}
	QString ThemeManager::getColorString(const QString& key) {
		QString color = p->Config->getString(key);
		if (color.startsWith("#")) {
			return color;
		}
		else {
			return "#ED1C24";
		}
	}
}