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
		if (data.getControllerType() == AStoryXController::ControllerType::Unknown) {
			if (errorCode) {
				*errorCode = RebuildErrorCode::InvalidControllerType;
			}
			return "";
		}
		QString rtn;
		rtn += rule->getHeader(data.getControllerType());
		rtn += data.getRequiredParameter().getContent();
		QStringList optParaPrefixes = rule->getOptionalParameterPrefixes(data.getControllerType());
		QStringList optParaNames = rule->getOptionalParameterNames(data.getControllerType());
		for (auto optPara : data.getOptionalParameters()) {
			qint32 index = optParaNames.indexOf(optPara.getName());
			if (index == -1) {
				if (errorCode) {
					*errorCode = RebuildErrorCode::InvalidOptionalParameter;
				}
				return "";
			}
			rtn += optParaPrefixes[index] + optPara.getContent();
		}
		if (errorCode) {
			*errorCode = RebuildErrorCode::Success;
		}
		return rtn;
	}

	QString AStoryXRebuilder::rebuild(const QString& ruleName, AStoryXController::ControllerType type, 
		const QString& reqPara, const QMap<QString, QString>& optParas, RebuildErrorCode* errorCode) {
		AStoryXRule* rule = AStoryXRule::getRule(ruleName);
		if (rule == nullptr) {
			if (errorCode) {
				*errorCode = RebuildErrorCode::InvalidRuleName;
			}
			return "";
		}
		if (type == AStoryXController::ControllerType::Unknown) {
			if (errorCode) {
				*errorCode = RebuildErrorCode::InvalidControllerType;
			}
			return "";
		}
		QString rtn;
		rtn += rule->getHeader(type);
		rtn += reqPara;
		QStringList optParaPrefixes = rule->getOptionalParameterPrefixes(type);
		QStringList optParaNames = rule->getOptionalParameterNames(type);
		for (auto it = optParas.begin(); it != optParas.end(); it++) {
			qint32 index = optParaNames.indexOf(it.key());
			if (index == -1) {
				if (errorCode) {
					*errorCode = RebuildErrorCode::InvalidOptionalParameter;
				}
				return "";
			}
			rtn += optParaPrefixes[index] + it.value();
		}
		if (errorCode) {
			*errorCode = RebuildErrorCode::Success;
		}
		return rtn;
	}

	QString AStoryXRebuilder::rebuild(const QString& ruleName, const QString& rawLine, 
		qint32 lineIndex, RebuildErrorCode* errorCode) {
		AStoryXRule* rule = AStoryXRule::getRule(ruleName);
		if (rule == nullptr) {
			if (errorCode) {
				*errorCode = RebuildErrorCode::InvalidRuleName;
			}
			return "";
		}
		auto parseData = rule->parseAStoryX(rawLine, -1, true, lineIndex);
		return rebuild(ruleName, parseData, errorCode);
	}
}