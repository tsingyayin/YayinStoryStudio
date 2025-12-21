#pragma once
#include "../Macro.h"
#include "Logger.h"
#include <QtCore/qobject.h>
#include "LoggerMsgHandler.h"
// Forward declarations
namespace Visindigo::General {
	class LoggerMsgHandler;
	class LoggerManagerPrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI LoggerManager :public QObject {
		Q_OBJECT
	signals:
		void logReceived(const QString& handlerName, Logger::Level level, const QString& message, const LogMetaData& metaData);
	private:
#ifdef DEBUG
		LoggerManager(Logger::Level threshold = Logger::Debug);
#else
		LoggerManager(Logger::Level threshold = Logger::Info);
#endif
	public:
		~LoggerManager();
		static LoggerManager* getInstance();
		void msgHandlerLog(LoggerMsgHandler* handler);
		void setGlobalLogLevel(Logger::Level level);
		void finalSave();
	private:
		LoggerManagerPrivate* d;
	};
};