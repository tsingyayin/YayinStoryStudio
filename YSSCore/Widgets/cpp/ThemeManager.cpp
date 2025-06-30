#include "../ThemeManager.h"
#include <QtGui/qcolor.h>
#include <QtCore/qfile.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include "../../Utility/JsonConfig.h"
#include "../StyleSheetTemplate.h"
#include "../../Utility/FileUtility.h"
#include "../../General/Log.h"

namespace YSSCore::Widgets {
	class ThemeManagerPrivate {
		friend class ThemeManager;
	protected:
		YSSCore::Utility::JsonConfig* Config = nullptr;
		QMap<QString, StyleSheetTemplate*> TemplateSS;
		static ThemeManager* Instance;
		void addStyleSheetTemplate(const QString& key, StyleSheetTemplate* templateSS) {
			if (TemplateSS.contains(key)) {
				delete TemplateSS[key];
			}
			TemplateSS[key] = templateSS;
		}
		QString getRawStyleSheet(const QString& key) {
			QStringList keys = key.split("::");
			if (keys.size() != 2) {
				return "";
			}
			if (TemplateSS.contains(keys[0])) {
				return TemplateSS[keys[0]]->getRawStyleSheet(keys[1]);
			}
			else {
				yErrorF << "StyleSheetTemplate not found for key:" << key;
				return QString();
			}
		}
		QString getStyleSheet(const QString& key, YSSCore::Utility::JsonConfig* config = nullptr, QWidget* getter = nullptr) {
			QStringList keys = key.split("::");
			if (keys.size() != 2) {
				return "";
			}
			if (TemplateSS.contains(keys[0])) {
				return TemplateSS[keys[0]]->getStyleSheet(keys[1], config, getter);
			}
			else {
				yErrorF << "StyleSheetTemplate not found for key:" << key;
				return QString();
			}
		}
		~ThemeManagerPrivate() {
			for(auto ss: TemplateSS) {
				delete ss;
			}
			if (Config != nullptr) {
				delete Config;
			}
		}
	};
	ThemeManager* ThemeManagerPrivate::Instance = nullptr;
	ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
		d = new ThemeManagerPrivate();
		d->Instance = this;
		ySuccessF << "Success!";
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
		StyleSheetTemplate* templateSS = new StyleSheetTemplate();
		templateSS->parse(all);
		d->addStyleSheetTemplate(templateSS->getTemplateID(), templateSS);
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
	QString ThemeManager::getRawStyleSheet(const QString& key) {
		return d->getRawStyleSheet(key);
	}
	QString ThemeManager::getStyleSheet(const QString& key, QWidget* getter) {
		return d->getStyleSheet(key, d->Config, getter);
	}
}