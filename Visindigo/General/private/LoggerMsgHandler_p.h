#pragma once
#include <QtCore/qstring.h>
#include "../Logger.h"

namespace Visindigo::General {
	class LoggerMsgHandler;
}

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
	};
}
