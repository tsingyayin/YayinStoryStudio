#pragma once
#include "../Macro.h"
// Forward declarations
class QString;
namespace Visindigo::General {
	class LoggerMsgHandler;
	class LoggerPrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI Logger {
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
// Global Macros
#define yDebug Visindigo::General::Logger::global()->debug()
#define yMessage Visindigo::General::Logger::global()->message()
#define yInfo Visindigo::General::Logger::global()->info()
#define yLog Visindigo::General::Logger::global()->log()
#define yNotice Visindigo::General::Logger::global()->notice()
#define ySuccess Visindigo::General::Logger::global()->success()
#define yWarning Visindigo::General::Logger::global()->warning()
#define yError Visindigo::General::Logger::global()->error()

#define yDebugF yDebug<<"("<<__FUNCTION__<<":"<<__LINE__<<"):"
#define yMessageF yMessage<<"("<<__FUNCTION__<<":"<<__LINE__<<"):"
#define yInfoF yInfo<<"("<<__FUNCTION__<<":"<<__LINE__<<"):"
#define yLogF yLog<<"("<<__FUNCTION__<<":"<<__LINE__<<"):"
#define yNoticeF yNotice<<"("<<__FUNCTION__<<":"<<__LINE__<<"):"
#define ySuccessF ySuccess<<"("<<__FUNCTION__<<":"<<__LINE__<<"):"
#define yWarningF yWarning<<"("<<__FUNCTION__<<":"<<__LINE__<<"):"
#define yErrorF yError<<"("<<__FUNCTION__<<":"<<__LINE__<<"):"
