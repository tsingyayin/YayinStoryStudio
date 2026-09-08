#ifndef Visindigo_General_LogCenter_h
#define Visindigo_General_LogCenter_h
#include "VICompileMacro.h"
#include "General/Logger.h"
#include <QtCore/qobject.h>
#include "LoggerMsgHandler.h"
// Forward declarations
namespace Visindigo::General {
	class Exception;
	class LoggerMsgHandler;
	class LogCenterPrivate;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI LogCenter :public QObject {
		Q_OBJECT
	signals:
		void logReceived(const QString& handlerName, Logger::Level level, const QString& message, const QString& consoleStr, const LogMetaData& metaData);
	private:
		LogCenter(Logger::Level threshold = Logger::Debug);
	public:
		~LogCenter();
		static LogCenter* getInstance();
		void msgHandlerLog(LoggerMsgHandler* handler);
		void setGlobalLogLevel(Logger::Level level);
		void finalSave();
		void generateCrashReport(const Exception& ex);
		QString generateHardwareInfo(bool debugOutput = true, bool containsNetworkInfo = false);
		QString getFormattedLogStr(LoggerMsgHandler* handler);
		QString getPlainLogStr(LoggerMsgHandler* handler);
		QString getStacktraceStr(LoggerMsgHandler* handler);
	private:
		LogCenterPrivate* d;
	};
};
#endif // Visindigo_General_LogCenter_h
