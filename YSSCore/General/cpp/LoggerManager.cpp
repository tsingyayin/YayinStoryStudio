#include "../LoggerManager.h"
#include "../LoggerMsgHandler.h"
#include <QtCore/qdatetime.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qtextstream.h>
namespace YSSCore::General {
	class LoggerManagerPrivate {
		friend LoggerManager;
	protected:
		static LoggerManager* Instance;
		Logger::Level threshold;
		QFile LogFile;
		QTextStream* Stream;
		quint16 cache = 30;
		quint16 current = 0;
		LoggerManagerPrivate() {
			QFile logFile;
			logFile.setFileName("./resource/logs/latest.yss.log");
			if (logFile.exists()) {
				QFileInfo fileInfo("./resource/logs/latest.yss.log");
				QDateTime birthTime = fileInfo.lastModified();
				QString fileName = "./resource/logs/" + birthTime.toString("yyyy-MM-dd_hh_mm_ss") + "-%1.yss.log";
				int i = 0;
				while (!logFile.rename(fileName.arg(QString::number(i)))) {
					i++;
				};
			}
			logFile.close();
			LogFile.setFileName("./resource/logs/latest.yss.log");
			LogFile.open(QIODevice::NewOnly | QIODevice::Text | QIODevice::Append);
			Stream = new QTextStream(&LogFile);
			Stream->setEncoding(QStringConverter::Utf8);
		}
		void log(QString line) {
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
		QString logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") + "] " +
			"[" + handler->getLogger()->getNamespace() + "]: " + handler->getMessage();
		qDebug().noquote().nospace() << logStr;
		d->log(logStr);
	}
	void LoggerManager::setGlobalLogLevel(Logger::Level level) {
		d->threshold = level;
	}
	void LoggerManager::finalSave() {
		d->save();
	}
}