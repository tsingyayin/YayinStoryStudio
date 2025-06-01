#pragma once
#include "../Macro.h"
class QString;
namespace YSSCore::General {
	class LoggerMsgHandler;
	class LoggerPrivate;
	class YSSCoreAPI Logger {
	public:
		enum Level {
			Debug = 1000,
			Message = 2000,
			Info = Message,
			Log = Message,
			Notice = 3000,
			Success = 4000,
			Warning = 5000,
			Error = 6000,
		};
	public:
#ifdef DEBUG
		Logger(const QString& nameSpace, Level threshold = Debug);
#else
		Logger(const QString& nameSpace, Level threshold = Message);
#endif
		Logger(const Logger& other);
		Logger(Logger&& other);
		~Logger();
		Logger& operator=(const Logger& other);
		Logger& operator=(Logger&& other);
		static Logger* global();
		LoggerMsgHandler debug();
		LoggerMsgHandler message();
		LoggerMsgHandler info();
		LoggerMsgHandler log();
		LoggerMsgHandler notice();
		LoggerMsgHandler success();
		LoggerMsgHandler warning();
		LoggerMsgHandler error();
		QString getNamespace();
	private:
		LoggerPrivate* d;
	};
}

#define yDebug YSSCore::General::Logger::global()->debug()
#define yMessage YSSCore::General::Logger::global()->message()
#define yInfo YSSCore::General::Logger::global()->info()
#define yLog YSSCore::General::Logger::global()->log()
#define yNotice YSSCore::General::Logger::global()->notice()
#define ySuccess YSSCore::General::Logger::global()->success()
#define yWarning YSSCore::General::Logger::global()->warning()
#define yError YSSCore::General::Logger::global()->error()

#define yDebugF yDebug<<"("<<__FILE__<<":"<<__LINE__<<": "<<__FUNCTION__<<"): "
#define yMessageF yMessage<<"("<<__FILE__<<":"<<__LINE__<<": "<<__FUNCTION__<<"): "
#define yInfoF yInfo<<"("<<__FILE__<<":"<<__LINE__<<": "<<__FUNCTION__<<"): "
#define yLogF yLog<<"("<<__FILE__<<":"<<__LINE__<<": "<<__FUNCTION__<<"): "
#define yNoticeF yNotice<<"("<<__FILE__<<":"<<__LINE__<<": "<<__FUNCTION__<<"): "
#define ySuccessF ySuccess<<"("<<__FILE__<<":"<<__LINE__<<": "<<__FUNCTION__<<"): "
#define yWarningF yWarning<<"("<<__FILE__<<":"<<__LINE__<<": "<<__FUNCTION__<<"): "
#define yErrorF yError<<"("<<__FILE__<<":"<<__LINE__<<": "<<__FUNCTION__<<"): "
