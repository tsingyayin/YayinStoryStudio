#pragma once
#include <Editor/TabCompleterProvider.h>

class AStoryTabCompleterProvider : public Visindigo::Editor::TabCompleterProvider
{
	Q_OBJECT;
public:
	AStoryTabCompleterProvider(QTextDocument* parent);
	virtual QList<Visindigo::Editor::TabCompleterItem> onTabComplete(int position, QString lineContent, QString wordContent) override;
};