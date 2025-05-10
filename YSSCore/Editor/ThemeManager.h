#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"

class QColor;
class QString;
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
		ThemeManagerPrivate* d;
	};
}

#define YSSTM YSSCore::Editor::ThemeManager::getInstance()