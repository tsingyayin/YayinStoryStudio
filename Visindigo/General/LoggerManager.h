#pragma once
#include "../Macro.h"
#include "Logger.h"
namespace Visindigo::General {
	class LoggerMsgHandler;
	class LoggerManagerPrivate;
	class VisindigoAPI LoggerManager {
	public:
#ifdef DEBUG
		LoggerManager(Logger::Level threshold = Logger::Debug);
#else
		LoggerManager(Logger::Level threshold = Logger::Info);
#endif
		LoggerManager(const LoggerManager& other) = delete;
		LoggerManager(LoggerManager&& other) = delete;
		LoggerManager& operator=(const LoggerManager& other) = delete;
		LoggerManager& operator=(LoggerManager&& other) = delete;
		~LoggerManager();
		static LoggerManager* getInstance();
		void msgHandlerLog(LoggerMsgHandler* handler);
		void setGlobalLogLevel(Logger::Level level);
		void finalSave();
	private:
		LoggerManagerPrivate* d;
	};
};