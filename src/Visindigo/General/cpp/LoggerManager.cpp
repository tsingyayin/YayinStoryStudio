#include "../LoggerManager.h"
#include "../LoggerMsgHandler.h"
#include <QtCore/qdatetime.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdir.h>
#include "../../Utility/Console.h"
#include "../../General/VIApplication.h"
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
		QString LogFileNameTimeFormat;
		QString LogTimeFormat;
		LoggerManagerPrivate() {
			LogFileNameTimeFormat = VIApp->getEnvConfig(VIApplication::LogFileNameTimeFormat).toString();
			LogTimeFormat = VIApp->getEnvConfig(VIApplication::LogTimeFormat).toString();
			QString birthTime = QDateTime::currentDateTime().toString(LogFileNameTimeFormat);
			QString LogFolderPath = VIApp->getEnvConfig(VIApplication::LogFolderPath).toString();
			LogFile.setFileName(LogFolderPath%"/"%birthTime%".log");
			QDir logDir(LogFolderPath);
			if (!logDir.exists()) {
				logDir.mkpath(".");
			}
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

	/*!
		\class Visindigo::General::LoggerManager
		\inheaderfile General/LoggerManager.h
		\brief LoggerManager是日志记录器管理器，用于管理全局日志对象并处理日志消息。
		\inmodule Visindigo 
		\ingroup VDebug
		\since Visindigo 0.13.0

		VDebug是一种用于在Qt开发中与QDebug提供相似功能的日志管理。
		它并不旨在取代QDebug，因为它比QDebug性能更差，但更灵活。
		例如，您可以监听LoggerManager::logReceived信号自由的将
		日志路由到别处。除此之外，VDebug默认同时输出到文件，不需单独
		实现文件输出功能。

		此类是VDebug的核心类，用于管理Logger实例并处理日志消息的输出。
		
		VDebug和QDebug在结构上略有不同：
		\list
		\li QDebug使用QDebug类来承接日志消息，而VDebug则是LoggerMsgHandler类。
		\li QDebug实例可以使用QMessageLogger类获取，而VDebug则是通过Logger类获取。
		\li QMessageLogger使用不同的QMessageLoggingCategory构造QDebug来区分不同的日志命名空间，
		而VDebug则是通过Logger类的实例来区分不同的日志命名空间。
		\endlist

		\section1 性能与特性说明
		在0.13.0之前的版本中，VDebug是重新转发到QDebug实现的输出，因此性能较差。

		在0.13.0开发过程中，曾经被直接使用std::cout实现输出，但这在Android平台上导致
		调试器看不到日志，并不实用。
		
		现在的实现中，已经改用Qt未公开的函数qt_message_output来实现日志输出，
		这使得VDebug的性能（至少在最后输出阶段）与QDebug相当。
	*/

	/*!
		\since Visindigo 0.13.0
		构造LoggerManager实例。
		\a threshold 全局日志级别阈值，低于此级别的日志将不会被输出。

		这是一个私有构造函数，用户无法直接调用此函数创建LoggerManager实例。
		要获取LoggerManager实例，请使用静态函数LoggerManager::getInstance()。
		在QT_DEBUG定义时，默认阈值为Logger::Debug，否则为Logger::Info。
	*/
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

	/*!
		\since Visindigo 0.13.0
		获取LoggerManager单例对象的指针。

		这是Visindigo单例类getInstance()函数的一个特例。其他单例类的getInstance()函数均会在实例不存在时创建它们。请注意区分。
	*/
	LoggerManager* LoggerManager::getInstance() {
		if (LoggerManagerPrivate::Instance == nullptr) {
			return new LoggerManager();
		}
		else {
			return LoggerManagerPrivate::Instance;
		}
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	LoggerManager::~LoggerManager() {
		delete d;
	}

	static inline QString getHandlerMsg(LoggerMsgHandler* handler) {
		QString result = (handler->getMetaData().isValid() ? QString("[") % handler->getMetaData().toString() % QString("]: ") : QString(""))
			% handler->getMessage();
		if ((int)handler->getLevel() >= (int)Logger::Level::Warning) {
			if (!handler->getStacktrace().isEmpty()) {
				result += "\n\tStacktrace:\n";
				quint32 index = 0;
				for (const StacktraceFrame& frame : handler->getStacktrace()) {
					result += QString("\t%1\t%2 ! %3 (+%4) in %5 at %6\n")
						.arg(QString::number(index))
						.arg(frame.getBinaryFileName())
						.arg(frame.getFunctionName())
						.arg(QString::number(frame.getAddress(), 16).toUpper())
						.arg(frame.getSourceFileName())
						.arg(QString::number(frame.getLineNumber()));
					index++;
				}
			}
		}
		return result;
	}
	/*!
		\since Visindigo 0.13.0
		处理日志消息。
		\a handler 指向LoggerMsgHandler对象的指针，包含了日志消息的相关信息。

		此函数由LoggerMsgHandler对象在其析构函数中自动调用，用于将日志消息输出到控制台和日志文件中。

		强烈不推荐用户手动调用此函数。
	*/
	void LoggerManager::msgHandlerLog(LoggerMsgHandler* handler) {
		// TODO:
		// This implementation does not check the log level threshold.
		// It will log all messages regardless of the threshold.
		QString logStr = QString("[") % QDateTime::currentDateTime().toString(d->LogTimeFormat) %
			"][" % handler->getLogger()->getNamespace() % "]";
		switch (handler->getLevel()) {
		case Logger::Level::Debug:
			logStr += "[DEBUG]" % getHandlerMsg(handler);
			qt_message_output(QtMsgType::QtDebugMsg, QMessageLogContext(), logStr);
			break;
		case Logger::Level::Message:
			logStr += "[MSG]" % getHandlerMsg(handler);
			qt_message_output(QtMsgType::QtInfoMsg, QMessageLogContext(), logStr);
			break;
		case Logger::Level::Notice:
			logStr += Visindigo::Utility::Console::inNoticeStyle("[NOTICE]" % getHandlerMsg(handler));
			qt_message_output(QtMsgType::QtInfoMsg, QMessageLogContext(), logStr);
			break;
		case Logger::Level::Success:
			logStr += Visindigo::Utility::Console::inSuccessStyle("[SUCCESS]" % getHandlerMsg(handler));
			qt_message_output(QtMsgType::QtInfoMsg, QMessageLogContext(), logStr);
			break;
		case Logger::Level::Warning:
			logStr += Visindigo::Utility::Console::inWarningStyle("[WARNING]" % getHandlerMsg(handler));
			qt_message_output(QtMsgType::QtWarningMsg, QMessageLogContext(), logStr);
			break;
		case Logger::Level::Error:
			logStr += Visindigo::Utility::Console::inErrorStyle("[ERROR]" % getHandlerMsg(handler));
			qt_message_output(QtMsgType::QtCriticalMsg, QMessageLogContext(), logStr);
			break;
		}
		d->log(Visindigo::Utility::Console::getRawText(logStr));
		emit logReceived(handler->getLogger()->getNamespace(), handler->getLevel(), handler->getMessage(), handler->getMetaData());
	}

	/*!
		\since Visindigo 0.13.0
		设置全局日志级别阈值。
		\a level 全局日志级别阈值，低于此级别的日志将不会被输出。

		此函数允许用户动态调整全局日志级别阈值。
	*/
	void LoggerManager::setGlobalLogLevel(Logger::Level level) {
		d->threshold = level;
	}

	/*!
		\since Visindigo 0.13.0
		将日志缓冲区的内容保存到日志文件中。

		此函数强制将日志缓冲区的内容写入日志文件中。
		通常情况下，日志会自动定期保存，但用户也可以手动调用此函数以确保日志被保存。
	*/
	void LoggerManager::finalSave() {
		d->save();
	}
}