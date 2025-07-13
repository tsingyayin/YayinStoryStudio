#pragma once
#include "../Macro.h"
#include "ASEParameter.h"
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <Macro.h>
namespace YSSCore::Utility {
	class JsonConfig;
}
class QString;

class AStoryControllerParseData;
class ASRuleControllerPrivate;
class AStoryController {
	VImpl(AStoryController);
public:
	enum Name {
		unknown = -1,
		talk = 1,
		bg,
		music,
		sound,
		cover,
		avg,
		flash,
		effect,
		shake,
		filter,
		delay,
		option,
		jump,
		sign,
		hologram,
		tele,
	};
public:
	static Name fromNameString(const QString& name);
	static QString toNameString(Name name);
public:
	AStoryController();
	AStoryController(const QString& name, YSSCore::Utility::JsonConfig& meta, const QString & rule);
	VIMoveable(AStoryController);
	VICopyable(AStoryController);
	void setControllerName(const QString& name);
	void setControllerASRule(const QString& rule);
	QString getStartSign();
	void setControllerMeta(YSSCore::Utility::JsonConfig& meta);
	QList<ASEParameter::Type> getDefaultParameterTypes();
	Name getControllerName();
	QStringList getDefaultParameterNames();
	AStoryControllerParseData parse(const QString& input);
};
