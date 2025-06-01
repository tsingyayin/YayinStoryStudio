#include "../ThemeManager.h"
#include <QtGui/qcolor.h>
#include <QtCore/qfile.h>
#include <QtCore/qiodevice.h>
#include "../../Utility/JsonConfig.h"
#include "../StyleSheetTemplate.h"
#include "../../Utility/FileUtility.h"

namespace YSSCore::Editor {
	class ThemeManagerPrivate {
		friend class ThemeManager;
	protected:
		YSSCore::Utility::JsonConfig* Config = nullptr;
		StyleSheetTemplate TemplateSS;
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
		QString all = YSSCore::Utility::FileUtility::readAll(path);
		if (d->Config != nullptr) {
			delete d->Config;
		}
		d->Config = new YSSCore::Utility::JsonConfig(all);
		if (d->Config->contains("Template")) {
			loadStyleTemplate("./resource/theme/template/" + d->Config->getString("Template") + ".yst");
		}
	}
	void ThemeManager::loadStyleTemplate(const QString& path) {
		QString all = YSSCore::Utility::FileUtility::readAll(path);
		d->TemplateSS.parse(all);
	}
	QString ThemeManager::getThemeName() {
		return d->Config->getString("ThemeName");
	}
	QString ThemeManager::getTemplateName(){
		return d->TemplateSS.getTemplateName();
	}
	QString ThemeManager::getThemeID() {
		return d->Config->getString("ThemeID");
	}
	QString ThemeManager::getTemplateID() {
		return d->TemplateSS.getTemplateID();
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
	QString ThemeManager::getRawStyleSheet(const QString& key) {
		return d->TemplateSS.getRawStyleSheet(key);
	}
	QString ThemeManager::getStyleSheet(const QString& key, QWidget* getter) {
		return d->TemplateSS.getStyleSheet(key, d->Config, getter);
	}
}