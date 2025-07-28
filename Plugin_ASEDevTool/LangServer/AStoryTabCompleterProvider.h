#pragma once
#include <Editor/TabCompleterProvider.h>

class AStoryTabCompleterProvider : public YSSCore::Editor::TabCompleterProvider
{
	Q_OBJECT;
public:
	AStoryTabCompleterProvider(QTextDocument* parent);
	virtual QList<YSSCore::Editor::TabCompleterItem> onTabComplete(int position, QString lineContent, QString wordContent) override;
};