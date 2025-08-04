#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"
// Forward declarations
class QWidget;
class QColor;
class QString;
namespace Visindigo::Widgets {
	class ThemeManagerPrivate;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI ThemeManager :public QObject
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
#define YSSTM Visindigo::Widgets::ThemeManager::getInstance()
#define YSSTMRSS Visindigo::Widgets::ThemeManager::getInstance()->getRawStyleSheet
#define YSSTMSS Visindigo::Widgets::ThemeManager::getInstance()->getStyleSheet