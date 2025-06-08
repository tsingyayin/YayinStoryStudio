#pragma once
#include <QtCore/qstring.h>
#include "../Logger.h"

namespace YSSCore::General {
	class LoggerMsgHandler;
}

namespace YSSCore::__Private__ {
	/*
		DO NOT USE THIS CLASS
	*/
	class LoggerMsgHandlerPrivate {
		friend class YSSCore::General::LoggerMsgHandler;
	protected:
		YSSCore::General::Logger* Logger;
		QString Msg;
		YSSCore::General::Logger::Level Level;
	};
}
