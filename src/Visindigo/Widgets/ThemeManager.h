#pragma once
#include <QtCore/qobject.h>
#include <QtGui/qstylehints.h>
#include "../Macro.h"
// Forward declarations
class QWidget;
class QColor;
class QString;
namespace Visindigo::Widgets {
	class ThemeManagerPrivate;
}
namespace Visindigo::General {
	class Plugin;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI ThemeManager :public QObject
	{
		Q_OBJECT;
		friend class NThemeManagerPrivate;
	signals:
		void systemThemeChanged(Qt::ColorScheme newScheme);
		void programThemeChanged(const QString& themeID);
	public:
		enum ThemeID {
			System = -1,
			Unknown = Qt::ColorScheme::Unknown,
			Light = Qt::ColorScheme::Light,
			Dark = Qt::ColorScheme::Dark,
			White, Black
		};
		static QString themeIDToString(ThemeID id);
		static ThemeID stringToThemeID(const QString& str);
		static void registerUserThemeID(const QString& id, ThemeID themeID);
	private:
		ThemeManager(QObject* parent = nullptr);
	public:
		static ThemeManager* getInstance();
		~ThemeManager();
		bool pluginRegisterColorScheme(Visindigo::General::Plugin* plugin, const QString& jsonStr);
		bool pluginRegisterStyleTemplate(Visindigo::General::Plugin* plugin, QString& vstStr);
		bool isColorSchemeFromPlugin(const QString& ID);
		bool isStyleTemplateFromPlugin(const QString& ID);
		void loadAndRefresh(bool autoMergeAndApply = true);
		bool addColorSchemeToPriority(const QString& ID); // scheme is group of themes, theme is group of colors
		bool addStyleTemplateToPriority(const QString& ID); // template is group of stylesheets
		bool removeColorSchemeFromPriority(const QString& ID);
		bool removeStyleTemplateFromPriority(const QString& ID);
		void clearColorSchemePriority();
		void clearStyleTemplatePriority();
		void setColorSchemePriority(const QStringList& schemeIDList);
		void setStyleTemplatePriority(const QStringList& templateIDList);
		void mergeAndApply();
		QStringList getAllColorSchemes();
		QStringList getAvailableColorSchemes();
		QStringList getAllStyleTemplates();
		QStringList getAvailableStyleTemplates();
		QStringList getColorThemes();
		QStringList getStyleNamespaces();
		bool changeColorTheme(const QString& themeID);
		void autoAdjustThemeToSystem(bool autoAdjust);
		QString getCurrentColorTheme();
		QColor getColor(const QString& key);
		QString getRawTemplate(const QString& key);
		QString getTemplate(const QString& key, QWidget* getter = nullptr);
	private:
		ThemeManagerPrivate* d;
	};
}
// Global Macros
#define VISTM Visindigo::Widgets::ThemeManager::getInstance()
#define VISTMGRT Visindigo::Widgets::ThemeManager::getInstance()->getRawTemplate
#define VISTMGT Visindigo::Widgets::ThemeManager::getInstance()->getTemplate