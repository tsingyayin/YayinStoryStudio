#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"
// Forward declarations
class QWidget;
class QColor;
class QString;
namespace YSSCore::Widgets {
	class ThemeManagerPrivate;
}
// Main
namespace YSSCore::Widgets {
	class YSSCoreAPI ThemeManager :public QObject
	{
		Q_OBJECT;
		friend class ThemeManagerPrivate;
	public:
		static ThemeManager* getInstance();
		ThemeManager(QObject* parent = nullptr);
		void loadConfig(const QString& path);
		void loadStyleTemplate(const QString& path);
		QString getThemeName();
		QString getThemeID();
		QColor getColor(const QString& key);
		QString getColorString(const QString& key);
		QString getRawStyleSheet(const QString& key);
		QString getStyleSheet(const QString& key, QWidget* getter = nullptr);
	private:
		ThemeManagerPrivate* d;
	};
}
// Global Macros
#define YSSTM YSSCore::Widgets::ThemeManager::getInstance()
#define YSSTMRSS YSSCore::Widgets::ThemeManager::getInstance()->getRawStyleSheet
#define YSSTMSS YSSCore::Widgets::ThemeManager::getInstance()->getStyleSheet