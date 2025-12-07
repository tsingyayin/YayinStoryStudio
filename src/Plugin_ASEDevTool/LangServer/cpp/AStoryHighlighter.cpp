#include "../AStoryHighlighter.h"
#include "../AStoryController.h"
#include "../AStoryControllerParseData.h"
#include "../ASRuleAdaptor.h"
#include <General/YSSProject.h>
#include <Utility/FileUtility.h>
#include <General/Log.h>
AStorySyntaxHighlighter::AStorySyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
	YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
	QString rootPath = project ? project->getProjectFolder() : "";
	QString rulePath = rootPath + "/Rules/StoryExplainer/BaseRule.asrule";
	RuleAdaptor = new ASRuleAdaptor(rulePath, ":/plugin/compiled/ASEDevTool/template/2.05.22.1A/RuleMeta.json");
	RuleAdaptor->loadASRule();
	Config = new Visindigo::Utility::JsonConfig();
	Config->parse(Visindigo::Utility::FileUtility::readAll(":/plugin/compiled/ASEDevTool/theme.json"));
}
void AStorySyntaxHighlighter::highlightBlock(const QString& text) {
	if (text.isEmpty()) {
		return; // Skip empty lines
	}
	else if (text == "------") {
		setFormatWithParaType(0, text.length(), "SyntaxSign");
	}
	else if (text.startsWith("//")) {
		setFormatWithParaType(0, text.length(), "Comment");
	}
	else {
		AStoryControllerParseData parseData = RuleAdaptor->parse(text);
		if (parseData.getControllerName() == AStoryController::Name::unknown) {
			return; // Skip unknown controllers
		}
		setFormatWithParaType(0, parseData.getStartSign().size(), "Controller." + AStoryController::toNameString(parseData.getControllerName()));
		if (parseData.getParameterCount() == 0) {
			return; // Skip if no parameters
		}
		int i = 0;
		if (parseData.getControllerName() == AStoryController::Name::sign) {
			setFormatWithParaType(parseData.getStartIndex()[0], parseData.getParameters()[0].length(), "Sign_T", "Sign_B");
			if (parseData.getParameterCount() == 2) {
				int startIndex = parseData.getStartIndex()[1];
				QString parameter = parseData.getParameters()[1];
				QStringList parts = parameter.split(",");
				int delta = 0;
				for (auto part : parts) {
					QStringList subParts = part.split(" ");
					if (subParts.size() == 2) {
						setFormatWithParaType(startIndex + delta, subParts[0].length(), "String");
						setFormatWithParaType(startIndex + delta + subParts[0].length() + 1, subParts[1].length(), "Number");
					}
					delta += part.length() + 1; // +1 for the ','
				}
			}
			return;
		}
		else if (parseData.getControllerName() == AStoryController::Name::branch) {
			for (; i < parseData.getParameterCount(); i++) {
				int startIndex = parseData.getStartIndex()[i];
				QString parameter = parseData.getParameters()[i];
				QStringList parts = parameter.split("->");
				if (parts.size() == 2) {
					setFormatWithParaType(startIndex, parts[0].length(), "String");
					setFormatWithParaType(startIndex + parts[0].length() + 2, parts[1].length(), "Sign_T", "Sign_B");
				}
			}
			return;
		}
		else if (parseData.getControllerName() == AStoryController::Name::jump) {
			setFormatWithParaType(parseData.getStartIndex()[0], parseData.getParameters()[0].length(), "Sign_T", "Sign_B");
			if (parseData.getParameterCount() == 2) {
				QStringList parts = parseData.getParameters()[1].split(" ");
				int delta = 0;
				int size_3 = (parts.size() / 3) * 3;
				for (int i = 0; i < size_3; i++) {
					switch (i % 3) {
					case 0: // Name
						setFormatWithParaType(parseData.getStartIndex()[1] + delta, parts[i].length(), "String");
						delta += parts[i].length() + 1; // +1 for the space
						break;
					case 1: // Value
						setFormatWithParaType(parseData.getStartIndex()[1] + delta, parts[i].length(), "Number");
						delta += parts[i].length() + 1; // +1 for the space
						break;
					case 2: // Sign
						setFormatWithParaType(parseData.getStartIndex()[1] + delta, parts[i].length(), "Sign_T", "Sign_B");
						delta += parts[i].length() + 1; // +1 for the space
						break;
					}
				}
				if (parts.size() > size_3) {
					// Handle the last part if it is not a complete triplet
					setFormatWithParaType(parseData.getStartIndex()[1] + delta, parts.last().length(), "Sign_T", "Sign_B");
				}
			}
			return;
		}
		if (parseData.getControllerName() == AStoryController::Name::speak) {
			setFormatWithParaType(parseData.getStartIndex()[0], parseData.getParameters()[0].length(), "Speaker");
			if (parseData.getParameterCount() > 1) {
				setFormatWithParaType(parseData.getStartIndex()[1], parseData.getParameters()[1].length(), "Normal");
			}
			i = 2; // Start from the third parameter
		}
		if (i >= parseData.getParameterCount()) {
			return; // No parameters to highlight
		}
		for (; i < parseData.getParameterCount(); i++) {
			int startIndex = parseData.getStartIndex()[i];
			QString parameter = parseData.getParameters()[i];
			ASEParameter::Type type = parseData.getParameterTypes()[i];
			//yDebugF << startIndex << parameter << ASEParameter::typeToString(type);
			setFormatWithParaType(startIndex, parameter.length(), ASEParameter::typeToString(type));
		}
	}
}
void AStorySyntaxHighlighter::setFormatWithParaType(int start, int length, QString type, QString type_B) {
	QString color = Config->getString("Dark.Types." + type);
	QColor formatColor(color);
	QTextCharFormat format;
	format.setForeground(formatColor);
	if (!type_B.isEmpty()) {
		format.setBackground(QColor(Config->getString("Dark.Types." + type_B)));
	}
	setFormat(start, length, format);
}