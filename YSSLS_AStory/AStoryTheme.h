#pragma once
#include <Utility/JsonConfig.h>

class AStoryLanguageServer;
class AStoryTheme {
private:
	YSSCore::Utility::JsonConfig* Config = nullptr;
	AStoryLanguageServer* Server = nullptr;
	static AStoryTheme* Instance;
public:
	static AStoryTheme* getInstance();
	AStoryTheme(AStoryLanguageServer* server);
	void loadTheme();
	QColor getColor(const QString& key);
	QString getColorString(const QString& key);
};