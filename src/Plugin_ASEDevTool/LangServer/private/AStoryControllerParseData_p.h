#pragma once
#include "../ASEParameter.h"
#include "../AStoryController.h"
#include <QtCore/qstringlist.h>
// Forward declarations
class AStoryController;
class AStoryControllerPrivate;
class AStoryControllerParseData;
// Main
class AStoryControllerParseDataPrivate {
	friend class AStoryControllerParseData;
	friend class AStoryControllerPrivate;
	friend class AStoryController;
protected:
	AStoryController::Name ControllerName = AStoryController::unknown;
	QString StartSign;
	QList<int> StartIndex;
	QList<ASEParameter::Type> ParameterTypes;
	QStringList Parameters;
	QStringList ParameterNames;
	QString CursorInWhichParameter;

	void addStartIndex(int index) {
		StartIndex.append(index);
	}
	void addParameterType(ASEParameter::Type type) {
		ParameterTypes.append(type);
	}
	void setParameterTypes(const QList<ASEParameter::Type>& types) {
		ParameterTypes = types;
	}
	void addParameter(const QString& parameter) {
		Parameters.append(parameter);
	}
	void setParameters(const QStringList& parameters) {
		Parameters = parameters;
	}
	void setStartSign(const QString& sign) {
		StartSign = sign;
	}
	void setParameterNames(const QStringList& names) {
		ParameterNames = names;
	}
	void clearParseData() {
		Parameters.clear();
		StartIndex.clear();
		CursorInWhichParameter.clear();
	}
};