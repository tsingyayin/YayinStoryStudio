#pragma once
#include <QtCore>
#include "../Utility/JsonConfig.h"
#include "../Macro.h"

namespace YSSCore::Editor {
	class ThemeManagerPrivate;
	class YSSCoreAPI ThemeManager :public QObject
	{
		Q_OBJECT;
		friend class ThemeManagerPrivate;
	public:
		static ThemeManager* getInstance();
		ThemeManager(QObject* parent = nullptr);
		void loadConfig(const QString& path);
		QString getThemeName();
		QString getThemeID();
		QColor getColor(const QString& key);
		QString getColorString(const QString& key);
	private:
		ThemeManagerPrivate* p;
	};
}

#define YSSTM YSSCore::Editor::ThemeManager::getInstance()