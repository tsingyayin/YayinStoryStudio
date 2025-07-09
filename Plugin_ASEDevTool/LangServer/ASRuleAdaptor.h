#pragma once
#include "../Macro.h"
#include <QtCore/qstringlist.h>
#include <Macro.h>
namespace YSSCore::Utility {
	class JsonConfig;
}
class QString;

class ASRuleSingleControllerPrivate;
class ASRuleSingleController {
	VImpl(ASRuleSingleController);
public:
	ASRuleSingleController();
	ASRuleSingleController(const QString & name, const YSSCore::Utility::JsonConfig& meta, const QString & rule);
	void setControllerName(const QString & name);
	void setControllerASRule(const QString & rule);
	QString getStartSign();
	void setControllerMeta(const YSSCore::Utility::JsonConfig & meta);
	QStringList parse(const QString& input);
};

class ASRuleAdaptorPrivate;
class ASRuleAdaptor {
	VImpl(ASRuleAdaptor);
public:
	ASRuleAdaptor();
	~ASRuleAdaptor();
	void setRulePath(const QString& path);

};