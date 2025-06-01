#include "../Logger.h"
#include <QtCore/qstring.h>
#include "../LoggerMsgHandler.h"
namespace YSSCore::General {
	class LoggerPrivate {
		friend class Logger;
	protected:
		Logger::Level Threshold;
		QString Namespace;
		static Logger* DefaultLogger;
	};
	Logger* LoggerPrivate::DefaultLogger = nullptr;

	Logger::Logger(const QString& nameSpace, Level threshold) {
		d = new LoggerPrivate;
		d->Namespace = nameSpace;
		d->Threshold = threshold;
	}
	Logger::Logger(const Logger& other) {
		d = new LoggerPrivate;
		d->Namespace = other.d->Namespace;
		d->Threshold = other.d->Threshold;
	}
	Logger::Logger(Logger&& other) {
		d = other.d;
		other.d = nullptr;
	}
	Logger::~Logger() {
		if (d != nullptr) {
			delete d;
		}
	}
	Logger& Logger::operator=(const Logger& other) {
		d->Namespace = other.d->Namespace;
		d->Threshold = other.d->Threshold;
		return *this;
	}
	Logger& Logger::operator=(Logger&& other) {
		if (d != nullptr) {
			delete d;
		}
		d = other.d;
		other.d = nullptr;
		return *this;
	}
	Logger* Logger::global() {
		if (LoggerPrivate::DefaultLogger == nullptr) {
			LoggerPrivate::DefaultLogger = new Logger("Global", Logger::Level::Debug);
		}
		return LoggerPrivate::DefaultLogger;
	}
	LoggerMsgHandler Logger::debug() {
		return LoggerMsgHandler(this, Logger::Level::Debug);
	}
	LoggerMsgHandler Logger::message() {
		return LoggerMsgHandler(this, Logger::Level::Message);
	}
	LoggerMsgHandler Logger::info() {
		return LoggerMsgHandler(this, Logger::Level::Info);
	}
	LoggerMsgHandler Logger::log() {
		return LoggerMsgHandler(this, Logger::Level::Log);
	}
	LoggerMsgHandler Logger::notice() {
		return LoggerMsgHandler(this, Logger::Level::Notice);
	}
	LoggerMsgHandler Logger::success() {
		return LoggerMsgHandler(this, Logger::Level::Success);
	}
	LoggerMsgHandler Logger::warning() {
		return LoggerMsgHandler(this, Logger::Level::Warning);
	}
	LoggerMsgHandler Logger::error() {
		return LoggerMsgHandler(this, Logger::Level::Error);
	}
	QString Logger::getNamespace() {
		return d->Namespace;
	}
}