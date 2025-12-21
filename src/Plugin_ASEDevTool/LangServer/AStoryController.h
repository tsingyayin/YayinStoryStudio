#pragma once
#include "../Macro.h"
#include "ASEParameter.h"
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <VIMacro.h>
namespace Visindigo::Utility {
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
		speak = 1,
		bg,
		music,
		sound,
		mask,
		chara,
		flash,
		effect,
		shake,
		filter,
		delay,
		branch,
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
	AStoryController(const QString& name, Visindigo::Utility::JsonConfig& meta, const QString& rule);
	VIMoveable(AStoryController);
	VICopyable(AStoryController);
	void setControllerName(const QString& name);
	void setControllerASRule(const QString& rule);
	QString getStartSign();
	void setControllerMeta(Visindigo::Utility::JsonConfig& meta);
	QList<ASEParameter::Type> getDefaultParameterTypes();
	Name getControllerName();
	QStringList getDefaultParameterNames();
	AStoryControllerParseData parse(const QString& input, qint32 cursorPos = -1);
};
