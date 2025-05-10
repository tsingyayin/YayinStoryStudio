#include "../ThemeManager.h"
#include <QtGui/qcolor.h>
#include <QtCore/qfile.h>
#include <QtCore/qiodevice.h>
#include "../../Utility/JsonConfig.h"
namespace YSSCore::Editor {
	class ThemeManagerPrivate {
		friend class ThemeManager;
	protected:
		YSSCore::Utility::JsonConfig* Config = nullptr;
		static ThemeManager* Instance;
	};
	ThemeManager* ThemeManagerPrivate::Instance = nullptr;
	ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
		d = new ThemeManagerPrivate();
		d->Instance = this;
	}
	ThemeManager* ThemeManager::getInstance() {
		return ThemeManagerPrivate::Instance;
	}
	void ThemeManager::loadConfig(const QString& path) {
		QFile file(path);
		if (file.open(QIODevice::ReadOnly)) {
			if (d->Config != nullptr) {
				delete d->Config;
			}
			d->Config = new YSSCore::Utility::JsonConfig(file.readAll());
			file.close();
		}
		else {
			qDebug() << "ThemeManager: loadConfig failed!";
		}
	}
	QString ThemeManager::getThemeName() {
		return d->Config->getString("ThemeName");
	}
	QString ThemeManager::getThemeID() {
		return d->Config->getString("ThemeID");
	}
	QColor ThemeManager::getColor(const QString& key) {
		QString color = d->Config->getString(key);
		if (color.startsWith("#")) {
			return QColor(color);
		}
		else {
			return QColor("#ED1C24");
		}
	}
	QString ThemeManager::getColorString(const QString& key) {
		QString color = d->Config->getString(key);
		if (color.startsWith("#")) {
			return color;
		}
		else {
			return "#ED1C24";
		}
	}
}