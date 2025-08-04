#include "../LoggerManager.h"
#include "../LoggerMsgHandler.h"
#include <QtCore/qdatetime.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qtimer.h>
#include "../../Utility/Console.h"

namespace Visindigo::General {
	class LoggerManagerPrivate {
		friend LoggerManager;
	protected:
		static LoggerManager* Instance;
		Logger::Level threshold;
		QFile LogFile;
		QTextStream* Stream;
		quint16 cache = 30;
		quint16 current = 0;
		QTimer Timer;
		bool Pause = false;
		LoggerManagerPrivate() {
			QString birthTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh_mm_ss");
			LogFile.setFileName("./resource/logs/" + birthTime + ".yss.log");
			LogFile.open(QIODevice::NewOnly | QIODevice::Text);
			Stream = new QTextStream(&LogFile);
			Stream->setEncoding(QStringConverter::Utf8);
			Timer.setInterval(10000);
			QObject::connect(&Timer, &QTimer::timeout, [this]() {
				if (!Pause) {
					save();
					current = 0;
				}
				Pause = true;
				});
			Timer.start();
		}
		void log(QString line) {
			Pause = false;
			Timer.start();
			*Stream << line << "\n";
			current++;
			if (current == cache) {
				current = 0;
				Stream->flush();
			}
		}
		void save() {
			Stream->flush();
		}
	};
	LoggerManager* LoggerManagerPrivate::Instance = nullptr;

	LoggerManager::LoggerManager(Logger::Level threshold) {
		if (LoggerManagerPrivate::Instance != nullptr) {
			d = LoggerManagerPrivate::Instance->d;
			LoggerManagerPrivate::Instance->d = nullptr;
			delete LoggerManagerPrivate::Instance;
			LoggerManagerPrivate::Instance = this;
		}
		else {
			d = new LoggerManagerPrivate;
			LoggerManagerPrivate::Instance = this;
		}
		LoggerManagerPrivate::Instance->setGlobalLogLevel(threshold);
	}

	LoggerManager* LoggerManager::getInstance() {
		if (LoggerManagerPrivate::Instance == nullptr) {
			return new LoggerManager();
		}
		else {
			return LoggerManagerPrivate::Instance;
		}
	}
	LoggerManager::~LoggerManager() {
		delete d;
	}
	void LoggerManager::msgHandlerLog(LoggerMsgHandler* handler) {
		// TODO:
		// This implementation does not check the log level threshold.
		// It will log all messages regardless of the threshold.
		QString logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") + "]" +
			"[" + handler->getLogger()->getNamespace() + "]";
		switch (handler->getLevel()) {
		case Logger::Level::Debug:
			logStr += "[DEBUG] " + handler->getMessage();
			break;
		case Logger::Level::Message:
			logStr += "[MSG] " + handler->getMessage();
			break;
		case Logger::Level::Notice:
			logStr += Visindigo::Utility::Console::inNoticeStyle("[NOTICE] " + handler->getMessage());
			break;
		case Logger::Level::Success:
			logStr += Visindigo::Utility::Console::inSuccessStyle("[SUCCESS] " + handler->getMessage());
			break;
		case Logger::Level::Warning:
			logStr += Visindigo::Utility::Console::inWarningStyle("[WARNING] " + handler->getMessage());
			break;
		case Logger::Level::Error:
			logStr += Visindigo::Utility::Console::inErrorStyle("[ERROR] " + handler->getMessage());
			break;
		}
		Visindigo::Utility::Console::print(logStr);
		d->log(Visindigo::Utility::Console::getRawText(logStr));
	}
	void LoggerManager::setGlobalLogLevel(Logger::Level level) {
		d->threshold = level;
	}
	void LoggerManager::finalSave() {
		d->save();
	}
}