#include "../AStoryControllerParseData.h"
#include "../private/AStoryControllerParseData_p.h"

AStoryControllerParseData::AStoryControllerParseData() {
	d = new AStoryControllerParseDataPrivate;
}

VIMoveable_Impl(AStoryControllerParseData);

VICopyable_Impl(AStoryControllerParseData);

AStoryController::Name AStoryControllerParseData::getControllerName() {
	return d->ControllerName;
}

QStringList AStoryControllerParseData::getParameters() {
	return d->Parameters;
}

QStringList AStoryControllerParseData::getParameterNames() {
	return d->ParameterNames;
}

QList<ASEParameter::Type> AStoryControllerParseData::getParameterTypes() {
	return d->ParameterTypes;
}

QList<int> AStoryControllerParseData::getStartIndex() {
	return d->StartIndex;
}

qint32 AStoryControllerParseData::getParameterCount() {
	return d->Parameters.size();
}

QString AStoryControllerParseData::toString() {
	int size = d->Parameters.size();
	QString rtn = "AStoryControllerParseData: %1\n";
	rtn = rtn.arg(AStoryController::toNameString(d->ControllerName));
	rtn += QString("Name-Types:\n");
	for (int i = 0; i < d->ParameterNames.size(); i++) {
		rtn += QString("%1: %2\n").arg(d->ParameterNames[i], ASEParameter::typeToString(d->ParameterTypes[i]));
	}
	for (int i = 0; i < size; i++) {
		rtn += QString("Parameter %1: \t%2, \tType: %3, \tStart Index: %4, \tEnd Index: %5\n")
			.arg(d->ParameterNames[i])
			.arg(d->Parameters[i])
			.arg(ASEParameter::typeToString(d->ParameterTypes[i]))
			.arg(d->StartIndex[i])
			.arg(d->StartIndex[i] + d->Parameters[i].length());
	}
	return rtn;
}

QString AStoryControllerParseData::getStartSign() {
	return d->StartSign;
}

QString AStoryControllerParseData::getCursorInWhichParameter() {
	return d->CursorInWhichParameter;
}