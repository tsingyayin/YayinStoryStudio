#include "../ASRuleAdaptor.h"
#include "../AStoryController.h"
#include "../AStoryControllerParseData.h"
#include <QtCore/qmap.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>

class ASRuleAdaptorPrivate {
	friend class ASRuleAdaptor;
protected:
	QString ASRulePath;
	QString MetaPath;
	QMap<QString, AStoryController> Controllers;
	AStoryController TalkController; // this controller does not have start sign
	YSSCore::Utility::JsonConfig Config;
};
ASRuleAdaptor::ASRuleAdaptor(const QString& asrulePath, const QString& metaPath)
	:d(new ASRuleAdaptorPrivate())
{
	setASRulePath(asrulePath);
	setMetaPath(metaPath);
}

ASRuleAdaptor::~ASRuleAdaptor() {
	delete d;
}

VIMoveable_Impl(ASRuleAdaptor);

VICopyable_Impl(ASRuleAdaptor);

void ASRuleAdaptor::setASRulePath(const QString& asrulePath) {
	d->ASRulePath = asrulePath;
}

void ASRuleAdaptor::setMetaPath(const QString& metaPath) {
	d->MetaPath = metaPath;
}

bool ASRuleAdaptor::loadASRule() {
	QJsonParseError status = d->Config.parse(YSSCore::Utility::FileUtility::readAll(d->MetaPath));
	if (status.error != QJsonParseError::NoError) {
		return false;
	}
	QStringList ControllerLines = YSSCore::Utility::FileUtility::readLines(d->ASRulePath);
	if (ControllerLines.isEmpty()) {
		return false;
	}
	AStoryController talkController;
	talkController.setControllerName(AStoryController::toNameString(AStoryController::talk));
	talkController.setControllerMeta(d->Config);
	talkController.setControllerASRule(ControllerLines.first());
	d->TalkController = talkController;
	for (int i = 1; i < ControllerLines.size(); i++) {
		AStoryController controller;
		controller.setControllerName(AStoryController::toNameString(static_cast<AStoryController::Name>(i + 1)));
		controller.setControllerMeta(d->Config);
		controller.setControllerASRule(ControllerLines.at(i));
		d->Controllers.insert(controller.getStartSign(), controller);
	}
	return true;
}
AStoryControllerParseData ASRuleAdaptor::parse(const QString& input) {
	AStoryControllerParseData parseData;
	for (auto Controller: d->Controllers) {
		if (input.startsWith(Controller.getStartSign())) {
			parseData = Controller.parse(input);
			return parseData;
		}
	}
	parseData = d->TalkController.parse(input);
	return parseData;
}