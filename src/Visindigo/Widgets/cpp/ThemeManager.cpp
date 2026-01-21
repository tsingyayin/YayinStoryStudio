#include "../ThemeManager.h"
#include <QtGui/qcolor.h>
#include <QtCore/qfile.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include "Utility/JsonConfig.h"
#include "Widgets/StyleSheetTemplate.h"
#include "Utility/FileUtility.h"
#include "General/Log.h"
#include "General/VIApplication.h"
#include "General/Plugin.h"
#include <QtWidgets/qapplication.h>

namespace Visindigo::Widgets {
	class ThemeManagerPrivate {
		friend class ThemeManager;
	protected:
		QString ConfigPath;
		Visindigo::Utility::JsonConfig* Config = nullptr;
		QMap<QString, StyleSheetTemplate> Templates;
		QMap<QString, Visindigo::Utility::JsonConfig*> ColorSchemes;
		QMap<QString, Visindigo::General::Plugin*> PluginMapForSchemes;
		QMap<QString, Visindigo::General::Plugin*> PluginMapForTemplates;
		QStringList ColorSchemePriority;
		QStringList StyleTemplatePriority;
		QString CurrentThemeID;
		StyleSheetTemplate MergedTemplate;
		Visindigo::Utility::JsonConfig MergedColorScheme;
		bool AutoAdjustToSystem = true;
		static ThemeManager* Instance;
		static QMap<QString, ThemeManager::ThemeID> UserThemeIDEnumMap;
	};

	QMap<QString, ThemeManager::ThemeID> ThemeManagerPrivate::UserThemeIDEnumMap = QMap<QString, ThemeManager::ThemeID>();
	QString ThemeManager::themeIDToString(ThemeID id) {
		switch (id) {
		case ThemeID::System:
			return "System";
		case ThemeID::Unknown:
			return "Unknown";
		case ThemeID::Light:
			return "Light";
		case ThemeID::Dark:
			return "Dark";
		case ThemeID::White:
			return "White";
		case ThemeID::Black:
			return "Black";
		default:
			for (auto it = ThemeManagerPrivate::UserThemeIDEnumMap.begin(); it != ThemeManagerPrivate::UserThemeIDEnumMap.end(); ++it) {
				if (it.value() == id) {
					return it.key();
				}
			}
			return "Unknown";
		}
	}

	ThemeManager::ThemeID ThemeManager::stringToThemeID(const QString& str) {
		if (str == "System") {
			return ThemeID::System;
		}
		else if (str == "Unknown") {
			return ThemeID::Unknown;
		}
		else if (str == "Light") {
			return ThemeID::Light;
		}
		else if (str == "Dark") {
			return ThemeID::Dark;
		}
		else if (str == "White") {
			return ThemeID::White;
		}
		else if (str == "Black") {
			return ThemeID::Black;
		}
		else {
			if (ThemeManagerPrivate::UserThemeIDEnumMap.contains(str)) {
				return ThemeManagerPrivate::UserThemeIDEnumMap[str];
			}
			else {
				return ThemeID::Unknown;
			}
		}
	}

	void ThemeManager::registerUserThemeID(const QString& id, ThemeID themeID) {
		ThemeManagerPrivate::UserThemeIDEnumMap[id] = themeID;
	}

	ThemeManager* ThemeManagerPrivate::Instance = nullptr;

	ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
		d = new ThemeManagerPrivate();
		d->ConfigPath = VIApp->getEnvConfig(Visindigo::General::VIApplication::EnvKey::ThemeFolderPath).toString();
		d->Config = new Visindigo::Utility::JsonConfig();
		if (!Visindigo::Utility::FileUtility::isFileExist(d->ConfigPath + "/config.json")) {
			vgWarningF << "Theme config file not found at path:" << d->ConfigPath + "/config.json" << ", creating default config.";
			d->Config->setArray("Schemes", QStringList());
			d->Config->setArray("Templates", QStringList());
			d->Config->setString("Theme", "Dark");
			Visindigo::Utility::FileUtility::saveAll(d->ConfigPath + "/config.json", d->Config->toString());
		}
		else {
			d->Config->parse(Visindigo::Utility::FileUtility::readAll(d->ConfigPath + "/config.json"));
			if (d->Config->contains("Schemes")) {
				QStringList schemes = d->Config->getStringList("Schemes");
				setColorSchemePriority(schemes);
			}
			if (d->Config->contains("Templates")) {
				QStringList templates = d->Config->getStringList("Templates");
				setStyleTemplatePriority(templates);
			}
			if (d->Config->contains("Theme")) {
				d->CurrentThemeID = d->Config->getString("Theme");
			}
			else {
				d->CurrentThemeID = "Dark";
			}
		}
		loadAndRefresh(false); // not merge for constructor
		connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme newScheme) {
			emit systemThemeChanged(newScheme);
			if (d->AutoAdjustToSystem) {
				QString targetThemeID = themeIDToString((ThemeID)(newScheme));
				if (targetThemeID != d->CurrentThemeID) {
					changeColorTheme(targetThemeID);
				}
			}
			});
		vgSuccessF << "Success!";
	}

	ThemeManager::~ThemeManager() {
		delete d;
	}

	ThemeManager* ThemeManager::getInstance() {
		if (ThemeManagerPrivate::Instance == nullptr) {
			ThemeManagerPrivate::Instance = new ThemeManager();
		}
		return ThemeManagerPrivate::Instance;
	}

	bool ThemeManager::pluginRegisterColorScheme(Visindigo::General::Plugin* plugin, const QString& jsonStr) {
		Visindigo::Utility::JsonConfig* json = new Visindigo::Utility::JsonConfig();
		if (json->parse(jsonStr).error == QJsonParseError::NoError) {
			if (!json->contains("SchemeID")) {
				vgWarningF << "Color scheme from plugin:" << plugin->getPluginName() << "does not contain SchemeID.";
				delete json;
				return false;
			}
			if (!json->contains("Schemes")) {
				vgWarningF << "Color scheme from plugin:" << plugin->getPluginName() << "does not contain Themes.";
				delete json;
				return false;
			}
			if (!json->contains("SchemeName")) {
				vgWarningF << "Color scheme from plugin:" << plugin->getPluginName() << "does not contain SchemeName.";
				delete json;
				return false;
			}
			QString schemeID = json->getString("SchemeID");
			d->ColorSchemes[schemeID] = json;
			d->PluginMapForSchemes[schemeID] = plugin;
			return true;
		}
		else {
			vgWarningF << "Failed to parse color scheme from plugin:" << plugin->getPluginName();
			delete json;
			return false;
		}
	}

	bool ThemeManager::pluginRegisterStyleTemplate(Visindigo::General::Plugin* plugin, QString& vstStr) {
		StyleSheetTemplate templateSS;
		if (templateSS.parse(vstStr)) {
			d->Templates[templateSS.getTemplateID()] = templateSS;
			d->PluginMapForTemplates[templateSS.getTemplateID()] = plugin;
			return true;
		}
		else {
			vgWarningF << "Failed to parse style template from plugin:" << plugin->getPluginName();
			return false;
		}
	}

	bool ThemeManager::isColorSchemeFromPlugin(const QString& ID) {
		return d->PluginMapForSchemes.contains(ID);
	}

	bool ThemeManager::isStyleTemplateFromPlugin(const QString& ID) {
		return d->PluginMapForTemplates.contains(ID);
	}

	void ThemeManager::loadAndRefresh(bool autoMergeAndApply) {
		d->Templates.clear();
		for (auto json : d->ColorSchemes) {
			delete json;
		}
		d->ColorSchemes.clear();

		QStringList templates = Visindigo::Utility::FileUtility::fileFilter(d->ConfigPath, { ".yst" });
		QStringList schemes = Visindigo::Utility::FileUtility::fileFilter(d->ConfigPath, { ".json" });

		for(auto name : templates) {
			QString all = Visindigo::Utility::FileUtility::readAll(name);
			StyleSheetTemplate templateSS;
			if (templateSS.parse(all)) {
				d->Templates[templateSS.getTemplateID()] = templateSS;
			}
			else {
				vgWarningF << "Failed to parse style template at path:" << name;
			}
		}

		for (auto name : schemes) {
			QString all = Visindigo::Utility::FileUtility::readAll(name);
			Visindigo::Utility::JsonConfig* json = new Visindigo::Utility::JsonConfig();
			if (json->parse(all).error == QJsonParseError::NoError) {
				if (!json->contains("SchemeID")) {
					vgWarningF << "Color scheme at path:" << name << "does not contain SchemeID.";
					delete json;
					continue;
				}
				if (!json->contains("Schemes")) {
					vgWarningF << "Color scheme at path:" << name << "does not contain Themes.";
					delete json;
					continue;
				}
				if (!json->contains("SchemeName")) {
					vgWarningF << "Color scheme at path:" << name << "does not contain SchemeName.";
					delete json;
					continue;
				}
				d->ColorSchemes[json->getString("SchemeID")] = json;
			}
			else {
				vgWarningF << "Failed to parse color scheme at path:" << name;
				delete json;
			}
		}
		if (autoMergeAndApply){
			mergeAndApply();
		}
	}

	bool ThemeManager::addColorSchemeToPriority(const QString& ID) {
		if (!d->ColorSchemes.contains(ID)) {
			vgWarningF << "Color scheme" << ID << "not found.";
			return false;
		}
		if (d->ColorSchemePriority.contains(ID)) {
			vgWarningF << "Color scheme" << ID << "already added.";
			return false;
		}
		d->ColorSchemePriority.append(ID);
		return true;
	}

	bool ThemeManager::addStyleTemplateToPriority(const QString& ID) {
		if (!d->Templates.contains(ID)) {
			vgWarningF << "Style template" << ID << "not found.";
			return false;
		}
		if (d->StyleTemplatePriority.contains(ID)) {
			vgWarningF << "Style template" << ID << "already added.";
			return false;
		}
		d->StyleTemplatePriority.append(ID);
		return true;
	}

	bool ThemeManager::removeColorSchemeFromPriority(const QString& ID) {
		return d->ColorSchemePriority.removeAll(ID) > 0;
	}

	bool ThemeManager::removeStyleTemplateFromPriority(const QString& ID) {
		return d->StyleTemplatePriority.removeAll(ID) > 0;
	}

	void ThemeManager::clearColorSchemePriority() {
		d->ColorSchemePriority.clear();
	}

	void ThemeManager::clearStyleTemplatePriority() {
		d->StyleTemplatePriority.clear();
	}

	void ThemeManager::setColorSchemePriority(const QStringList& schemeIDList) {
		d->ColorSchemePriority.clear();
		for (auto id : schemeIDList) {
			if (!d->ColorSchemes.contains(id)) {
				vgWarningF << "Color scheme" << id << "not found. Auto ignoring.";
				continue;
			}
			d->ColorSchemePriority.append(id);
		}
	}

	void ThemeManager::setStyleTemplatePriority(const QStringList& templateIDList) {
		d->StyleTemplatePriority.clear();
		for (auto id : templateIDList) {
			if (!d->Templates.contains(id)) {
				vgWarningF << "Style template" << id << "not found. Auto ignoring.";
				continue;
			}
			d->StyleTemplatePriority.append(id);
		}
	}

	void ThemeManager::mergeAndApply() {
		// Merge color schemes
		d->MergedColorScheme = Visindigo::Utility::JsonConfig();
		d->MergedColorScheme.setString("SchemeID", "#MergedColorScheme");
		d->MergedColorScheme.setString("SchemeName", "Final Merged Color Scheme");
		for (auto schemeID : d->ColorSchemePriority) {
			Visindigo::Utility::JsonConfig* scheme = d->ColorSchemes[schemeID];
			QStringList themes = scheme->keys("Themes");
			for (auto themeID : themes) {
				QStringList colorNames = scheme->keys("Themes." + themeID);
				for (auto colorName : colorNames) {
					d->MergedColorScheme.setString("Themes." + themeID + "." + colorName, scheme->getString("Themes." + themeID + "." + colorName));
				}
			}
		}
		// Merge style templates
		d->MergedTemplate = StyleSheetTemplate();
		d->MergedTemplate.setTemplateID("#MergedTemplate");
		d->MergedTemplate.setTemplateName("Final Merged Template");
		for (auto templateID : d->StyleTemplatePriority) {
			StyleSheetTemplate templateSS = d->Templates[templateID];
			d->MergedTemplate.merge(templateSS);
		}
		d->Config->setStringList("Schemes", d->ColorSchemePriority);
		d->Config->setStringList("Templates", d->StyleTemplatePriority);
		d->Config->setString("Theme", d->CurrentThemeID);
		Visindigo::Utility::FileUtility::saveAll(d->ConfigPath + "/config.json", d->Config->toString());
		emit programThemeChanged(d->Config->getString("Theme"));
	}

	QStringList ThemeManager::getAllColorSchemes() {
		return d->ColorSchemes.keys();
	}

	QStringList ThemeManager::getAvailableColorSchemes() {
		return d->ColorSchemePriority;
	}

	QStringList ThemeManager::getAllStyleTemplates() {
		return d->Templates.keys();
	}

	QStringList ThemeManager::getAvailableStyleTemplates() {
		return d->StyleTemplatePriority;
	}

	QStringList ThemeManager::getColorThemes() {
		return d->MergedColorScheme.keys();
	}

	QStringList ThemeManager::getStyleNamespaces() {
		return d->MergedTemplate.getNamespaces();
	}

	bool ThemeManager::changeColorTheme(const QString& themeID) {
		if (!d->MergedColorScheme.contains(themeID)) {
			vgWarningF << "ThemeID" << themeID << "not found in merged color scheme.";
			return false;
		}
		d->CurrentThemeID = themeID;
		emit programThemeChanged(themeID);
		return true;
	}

	void ThemeManager::autoAdjustThemeToSystem(bool autoAdjust) {
		d->AutoAdjustToSystem = autoAdjust;
	}

	QString ThemeManager::getCurrentColorTheme() {
		return d->CurrentThemeID;
	}

	QColor ThemeManager::getColor(const QString& key) {
		if (d->CurrentThemeID.isEmpty()) {
			vgWarningF << "No current theme set.";
			return QColor("#ED1C24");
		}
		Visindigo::Utility::JsonConfig theme = d->MergedColorScheme.getObject(d->CurrentThemeID);
		QString color = theme.getString("Colors." + key);
		if (color.startsWith("#")) {
			return QColor(color);
		}
		else {
			vgErrorF << "No such key" << key;
			return QColor("#ED1C24");
		}
	}

	QString ThemeManager::getRawTemplate(const QString& key) {
		return d->MergedTemplate.getRawStyleSheet(key);
	}

	QString ThemeManager::getTemplate(const QString& key, QWidget* getter) {
		return d->MergedTemplate.getStyleSheet(key, d->CurrentThemeID, &d->MergedColorScheme, getter);
	}
}