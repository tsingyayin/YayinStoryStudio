#pragma once
#include <QtCore/qstring.h>
#include "../Macro.h"
// Forward declarations

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
#ifdef QT_DEBUG
		explicit Logger(const QString& nameSpace, Level threshold = Debug);
#else
		explicit Logger(const QString& nameSpace, Level threshold = Message);
#endif
		Logger(const Logger& other) = delete;
		Logger(Logger&& other) = delete;
		~Logger();
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
		void setNamespace(const QString& nameSpace);
		Level getThreshold() const;
		void setThreshold(Level threshold);
	private:
		LoggerPrivate* d;
	};
}
// Global Macros
#define vDebug(getInstnaceFunction) getInstnaceFunction->debug()
#define vMessage(getInstnaceFunction) getInstnaceFunction->message()
#define vInfo(getInstnaceFunction) getInstnaceFunction->info()
#define vLog(getInstnaceFunction) getInstnaceFunction->log()
#define vNotice(getInstnaceFunction) getInstnaceFunction->notice()
#define vSuccess(getInstnaceFunction) getInstnaceFunction->success()
#define vWarning(getInstnaceFunction) getInstnaceFunction->warning()
#define vError(getInstnaceFunction) getInstnaceFunction->error()

#define vDebugF(getInstnaceFunction) getInstnaceFunction->debug()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)
#define vMessageF(getInstnaceFunction) getInstnaceFunction->message()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)
#define vInfoF(getInstnaceFunction) getInstnaceFunction->info()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)
#define vLogF(getInstnaceFunction) getInstnaceFunction->log()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)
#define vNoticeF(getInstnaceFunction) getInstnaceFunction->notice()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)
#define vSuccessF(getInstnaceFunction) getInstnaceFunction->success()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)
#ifdef VI_HAS_STD_STACKTRACE
#define vWarningF(getInstnaceFunction) getInstnaceFunction->warning()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)<<Visindigo::General::StacktraceHelper::getStacktrace()
#define vErrorF(getInstnaceFunction) getInstnaceFunction->error()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)<<Visindigo::General::StacktraceHelper::getStacktrace()

#define vDebugST(getInstnaceFunction) getInstnaceFunction->debug()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)<<Visindigo::General::StacktraceHelper::getStacktrace()
#define vMessageST(getInstnaceFunction) getInstnaceFunction->message()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)<<Visindigo::General::StacktraceHelper::getStacktrace()
#define vInfoST(getInstnaceFunction) getInstnaceFunction->info()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)<<Visindigo::General::StacktraceHelper::getStacktrace()
#define vLogST(getInstnaceFunction) getInstnaceFunction->log()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)<<Visindigo::General::StacktraceHelper::getStacktrace()
#define vNoticeST(getInstnaceFunction) getInstnaceFunction->notice()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)<<Visindigo::General::StacktraceHelper::getStacktrace()
#define vSuccessST(getInstnaceFunction) getInstnaceFunction->success()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)<<Visindigo::General::StacktraceHelper::getStacktrace()
#else
#define vWarningF(getInstnaceFunction) getInstnaceFunction->warning()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)
#define vErrorF(getInstnaceFunction) getInstnaceFunction->error()<<Visindigo::General::LogMetaData(__FUNCTION__, (qint32)__LINE__)
#define VDebugST(getInstnaceFunction) vDebugF(getInstnaceFunction)
#define VMessageST(getInstnaceFunction) vMessageF(getInstnaceFunction)
#define VInfoST(getInstnaceFunction) vInfoF(getInstnaceFunction)
#define VLogST(getInstnaceFunction) vLogF(getInstnaceFunction)
#define VNoticeST(getInstnaceFunction) vNoticeF(getInstnaceFunction)
#define VSuccessST(getInstnaceFunction) vSuccessF(getInstnaceFunction)
#endif

#define VILoggerGlobal Visindigo::General::Logger::global()

#define vgDebug vDebug(VILoggerGlobal)
#define vgMessage vMessage(VILoggerGlobal)
#define vgInfo vInfo(VILoggerGlobal)
#define vgLog vLog(VILoggerGlobal)
#define vgNotice vNotice(VILoggerGlobal)
#define vgSuccess vSuccess(VILoggerGlobal)
#define vgWarning vWarning(VILoggerGlobal)
#define vgError vError(VILoggerGlobal)

#define vgDebugF vDebugF(VILoggerGlobal)
#define vgMessageF vMessageF(VILoggerGlobal)
#define vgInfoF vInfoF(VILoggerGlobal)
#define vgLogF vLogF(VILoggerGlobal)
#define vgNoticeF vNoticeF(VILoggerGlobal)
#define vgSuccessF vSuccessF(VILoggerGlobal)
#define vgWarningF vWarningF(VILoggerGlobal)
#define vgErrorF vErrorF(VILoggerGlobal)

#define vgDebugST vDebugST(VILoggerGlobal)
#define vgMessageST vMessageST(VILoggerGlobal)
#define vgInfoST vInfoST(VILoggerGlobal)
#define vgLogST vLogST(VILoggerGlobal)
#define vgNoticeST vNoticeST(VILoggerGlobal)
#define vgSuccessST vSuccessST(VILoggerGlobal)

