#include "../AStoryTabCompleterProvider.h"

AStoryTabCompleterProvider::AStoryTabCompleterProvider(QTextDocument* parent)
	: YSSCore::Editor::TabCompleterProvider(parent) {

}

QList<YSSCore::Editor::TabCompleterItem> AStoryTabCompleterProvider::onTabComplete(int position, QString lineContent, QString wordContent) {
	QList<YSSCore::Editor::TabCompleterItem> items;
	YSSCore::Editor::TabCompleterItem test("", QString::number(position) + wordContent, lineContent, "");
	items.append(test);
	return items;
}