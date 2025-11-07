#pragma once
#include <VIMacro.h>
#include "AStoryController.h"
class QString;
class AStoryControllerParseData;

class ASRuleAdaptorPrivate;
class ASRuleAdaptor {
public:
	ASRuleAdaptor(const QString& asrulePath, const QString& metaPath);
	~ASRuleAdaptor();
	VIMoveable(ASRuleAdaptor);
	VICopyable(ASRuleAdaptor);
	void setASRulePath(const QString& asrulePath);
	void setMetaPath(const QString& metaPath);
	bool loadASRule();
	AStoryControllerParseData parse(const QString& input);
	QList<AStoryController> getAllControllers();
private:
	ASRuleAdaptorPrivate* d;
};