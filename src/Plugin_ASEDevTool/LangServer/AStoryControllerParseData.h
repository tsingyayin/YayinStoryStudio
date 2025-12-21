#pragma once
#include <QtCore/qlist.h>
#include "ASEParameter.h"
#include "AStoryController.h"
#include <VIMacro.h>
// Forward declarations
class AStoryController;
class AStoryControllerPrivate;
class AStoryControllerParseDataPrivate;
// Main
class AStoryControllerParseData
{
	friend class AStoryController;
	friend class AStoryControllerPrivate;
public:
	AStoryControllerParseData();
	VIMoveable(AStoryControllerParseData);
	VICopyable(AStoryControllerParseData);
	AStoryController::Name getControllerName();
	QStringList getParameters();
	QStringList getParameterNames();
	QList<ASEParameter::Type> getParameterTypes();
	qint32 getParameterCount();
	QList<int> getStartIndex();
	QString toString();
	QString getStartSign();
	QString getCursorInWhichParameter();
protected:
	AStoryControllerParseDataPrivate* d;
};
