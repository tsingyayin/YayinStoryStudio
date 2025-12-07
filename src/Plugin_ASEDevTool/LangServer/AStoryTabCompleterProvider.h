#pragma once
#include <Editor/TabCompleterProvider.h>

class ASRuleAdaptor;

class AStoryTabCompleterProvider : public YSSCore::Editor::TabCompleterProvider
{
	Q_OBJECT;
private:
	QList<YSSCore::Editor::TabCompleterItem> Controllers;
	ASRuleAdaptor* RuleAdaptor;
public:
	AStoryTabCompleterProvider(QTextDocument* parent);
	virtual QList<YSSCore::Editor::TabCompleterItem> onTabComplete(int position, QString lineContent, QString wordContent) override;
};