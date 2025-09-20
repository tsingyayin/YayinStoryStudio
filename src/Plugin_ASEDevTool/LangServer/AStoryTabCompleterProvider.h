#pragma once
#include <Editor/TabCompleterProvider.h>

class ASRuleAdaptor;

class AStoryTabCompleterProvider : public Visindigo::Editor::TabCompleterProvider
{
	Q_OBJECT;
private:
	QList<Visindigo::Editor::TabCompleterItem> Controllers;
	ASRuleAdaptor* RuleAdaptor;
public:
	AStoryTabCompleterProvider(QTextDocument* parent);
	virtual QList<Visindigo::Editor::TabCompleterItem> onTabComplete(int position, QString lineContent, QString wordContent) override;
};