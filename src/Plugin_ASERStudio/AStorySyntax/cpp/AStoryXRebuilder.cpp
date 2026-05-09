#include "AStorySyntax/AStoryXRebuilder.h"
#include "AStorySyntax/AStoryXRule.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
namespace ASERStudio::AStorySyntax {
	QString AStoryXRebuilder::rebuild(const QString& ruleName, const AStoryXControllerParseData& data, RebuildErrorCode* errorCode) {
		AStoryXRule* rule = AStoryXRule::getRule(ruleName);
		if (rule == nullptr) {
			if (errorCode) {
				*errorCode = RebuildErrorCode::InvalidRuleName;
			}
			return "";
		}
	}
}