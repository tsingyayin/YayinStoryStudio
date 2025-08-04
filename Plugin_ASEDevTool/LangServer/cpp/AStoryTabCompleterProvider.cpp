#include "../AStoryTabCompleterProvider.h"

AStoryTabCompleterProvider::AStoryTabCompleterProvider(QTextDocument* parent)
	: YSSCore::Editor::TabCompleterProvider(parent) {

}

QList<YSSCore::Editor::TabCompleterItem> AStoryTabCompleterProvider::onTabComplete(int position, QString lineContent, QString wordContent) {
	QList<YSSCore::Editor::TabCompleterItem> items;
	YSSCore::Editor::TabCompleterItem test("背景：", "背景：", "",YSSCore::Editor::TabCompleterItem::Function);
	items.append(test);
	YSSCore::Editor::TabCompleterItem test2("音效：", "音效：", "", YSSCore::Editor::TabCompleterItem::Function);
	items.append(test2);
	YSSCore::Editor::TabCompleterItem test3("[timestamp]", "[timestamp]", "", YSSCore::Editor::TabCompleterItem::Const);
	items.append(test3);
	YSSCore::Editor::TabCompleterItem test4("阿米娅", "阿米娅", "", YSSCore::Editor::TabCompleterItem::Value);
	items.append(test4);
	YSSCore::Editor::TabCompleterItem test5("天雨", "天雨", "", YSSCore::Editor::TabCompleterItem::Value);
	items.append(test5);
	return items;
}