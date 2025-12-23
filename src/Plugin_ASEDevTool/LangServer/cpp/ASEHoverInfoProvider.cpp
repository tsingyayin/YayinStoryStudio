#include "LangServer/ASEHoverInfoProvider.h"
#include "LangServer/ASRuleAdaptor.h"
#include "LangServer/AStoryControllerParseData.h"
#include <General/YSSProject.h>
#include <General/YSSLogger.h>

AStoryHoverInfoProvider::AStoryHoverInfoProvider(QTextDocument* doc)
	: YSSCore::Editor::HoverInfoProvider(doc)
{
	YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
	QString rootPath = project ? project->getProjectFolder() : "";
	QString rulePath = rootPath + "/Rules/StoryExplainer/BaseRule.asrule";
	RuleAdaptor = new ASRuleAdaptor(rulePath, ":/plugin/compiled/ASEDevTool/template/2.05.22.1A/RuleMeta.json");
	RuleAdaptor->loadASRule();
}

void AStoryHoverInfoProvider::onMouseHover(const QString& text, int position)
{
	if (triggerFromHover()) {
		if (position < 0 || position >= text.length()) {
			return;
		}
	}
	AStoryControllerParseData parseData = RuleAdaptor->parse(text, position);
	AStoryController::Name controllerName = parseData.getControllerName();
	QString md = "# "+RuleAdaptor->getParameterDocument(controllerName, "#title")%" ("+AStoryController::toNameString(controllerName)+")\n\n";
	md += RuleAdaptor->getParameterDocument(controllerName, "#description") + "\n\n";
	yDebugF << "Cursor in parameter:" << parseData.getCursorInWhichParameter();
	for (auto s : parseData.getParameterNames()) {
		if (s != parseData.getCursorInWhichParameter()) [[likly]]{
			md += " " + s + " ";
		}
		else {
			md += " <font color=#66CCFF>" + s + "</font> ";
		}
	}
	md += "\n\n" + RuleAdaptor->getParameterDocument(controllerName, parseData.getCursorInWhichParameter()); + "\n";
	
	yDebugF << md;
	setMarkdown(md);
}