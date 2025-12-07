#include "../AStoryTabCompleterProvider.h"
#include "../ASRuleAdaptor.h"
#include <General/YSSProject.h>
#include "../AStoryController.h"
AStoryTabCompleterProvider::AStoryTabCompleterProvider(QTextDocument* parent)
	: YSSCore::Editor::TabCompleterProvider(parent) {
	YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
	QString rootPath = project ? project->getProjectFolder() : "";
	QString rulePath = rootPath + "/Rules/StoryExplainer/BaseRule.asrule";
	RuleAdaptor = new ASRuleAdaptor(rulePath, ":/plugin/compiled/ASEDevTool/template/2.05.22.1A/RuleMeta.json");
	RuleAdaptor->loadASRule();
	for (AStoryController& controller : RuleAdaptor->getAllControllers()) {
		YSSCore::Editor::TabCompleterItem item(
			AStoryController::toNameString(controller.getControllerName()), controller.getStartSign(), "", YSSCore::Editor::TabCompleterItem::Function
		);
		Controllers.append(item);
	}
}

QList<YSSCore::Editor::TabCompleterItem> AStoryTabCompleterProvider::onTabComplete(int position, QString lineContent, QString wordContent) {
	QList<YSSCore::Editor::TabCompleterItem> items;
	if (position == 0 && lineContent.trimmed().isEmpty()) {
		items.append(Controllers);
	}
	return items;
}