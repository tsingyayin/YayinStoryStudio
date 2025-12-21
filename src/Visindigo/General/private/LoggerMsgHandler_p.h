#pragma once
#include <QtCore/qstring.h>
#include "General/Logger.h"
#include "General/LogMetaData.h"
#include "General/StacktraceHelper.h"

namespace Visindigo::__Private__ {
	/*
		DO NOT USE THIS CLASS
	*/
	class LoggerMsgHandlerPrivate {
		friend class Visindigo::General::LoggerMsgHandler;
	protected:
		Visindigo::General::Logger* Logger;
		QString Msg;
		Visindigo::General::Logger::Level Level;
		Visindigo::General::LogMetaData MetaData;
		QList<Visindigo::General::StacktraceFrame> Stacktrace;
	};
}
