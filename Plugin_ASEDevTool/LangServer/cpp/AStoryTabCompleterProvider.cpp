#include "../AStoryTabCompleterProvider.h"

AStoryTabCompleterProvider::AStoryTabCompleterProvider(QTextDocument* parent)
	: Visindigo::Editor::TabCompleterProvider(parent) {

}

QList<Visindigo::Editor::TabCompleterItem> AStoryTabCompleterProvider::onTabComplete(int position, QString lineContent, QString wordContent) {
	QList<Visindigo::Editor::TabCompleterItem> items;
	Visindigo::Editor::TabCompleterItem test("背景：", "背景：", "",Visindigo::Editor::TabCompleterItem::Function);
	items.append(test);
	Visindigo::Editor::TabCompleterItem test2("音效：", "音效：", "", Visindigo::Editor::TabCompleterItem::Function);
	items.append(test2);
	Visindigo::Editor::TabCompleterItem test3("[timestamp]", "[timestamp]", "", Visindigo::Editor::TabCompleterItem::Const);
	items.append(test3);
	Visindigo::Editor::TabCompleterItem test4("阿米娅", "阿米娅", "", Visindigo::Editor::TabCompleterItem::Value);
	items.append(test4);
	Visindigo::Editor::TabCompleterItem test5("天雨", "天雨", "", Visindigo::Editor::TabCompleterItem::Value);
	items.append(test5);
	Visindigo::Editor::TabCompleterItem test6("//", "注释", "", Visindigo::Editor::TabCompleterItem::Operator);
	items.append(test6);
	Visindigo::Editor::TabCompleterItem test7("Liner", "Liner", "", Visindigo::Editor::TabCompleterItem::Enum);
	items.append(test7);
	Visindigo::Editor::TabCompleterItem test8("普瑞赛斯", "普瑞赛斯", "", Visindigo::Editor::TabCompleterItem::Default);
	items.append(test8);
	return items;
}