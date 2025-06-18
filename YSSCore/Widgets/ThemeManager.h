#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"

class QWidget;
class QColor;
class QString;
namespace YSSCore::Widgets {
	class ThemeManagerPrivate;
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
		QString getTemplateName();
		QString getThemeID();
		QString getTemplateID();
		QColor getColor(const QString& key);
		QString getColorString(const QString& key);
		QString getRawStyleSheet(const QString& key);
		QString getStyleSheet(const QString& key, QWidget* getter = nullptr);
	private:
		ThemeManagerPrivate* d;
	};
}

#define YSSTM YSSCore::Widgets::ThemeManager::getInstance()