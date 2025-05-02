#pragma once
#include <QtCore>
#include "../Utility/JsonConfig.h"

namespace YSS{
	class ThemeManager :public QObject
	{
		Q_OBJECT;
	private:
		Utility::JsonConfig* Config = nullptr;
	public:
		ThemeManager(QObject* parent = nullptr);
		void loadConfig(const QString& path);
		QColor getColor(const QString& key);
		QString getColorString(const QString& key);
	};
}
