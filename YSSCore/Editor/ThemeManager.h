#pragma once
#include <QtCore>
#include "../Utility/JsonConfig.h"
#include "../Macro.h"

namespace YSSCore::Editor {
	class YSSCoreAPI ThemeManager :public QObject
	{
		Q_OBJECT;
	private:
		YSSCore::Utility::JsonConfig* Config = nullptr;
	public:
		ThemeManager(QObject* parent = nullptr);
		void loadConfig(const QString& path);
		QColor getColor(const QString& key);
		QString getColorString(const QString& key);
	};
}
