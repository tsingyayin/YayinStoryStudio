#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkinterface.h>
#include "General/Exception.h"
#include "General/LogCenter.h"
#include "General/LoggerMsgHandler.h"
#include "General/VIApplication.h"
#include "Utility/Console.h"
namespace Visindigo::General {
	class LogCenterPrivate {
		friend LogCenter;
	protected:
		static LogCenter* Instance;
		Logger::Level threshold = Logger::Info;
		QFile LogFile;
		QTextStream* Stream = nullptr;
		quint16 cache = 30;
		quint16 current = 0;
		QTimer Timer;
		bool Pause = false;
		QString LogFileNameTimeFormat;
		QString LogTimeFormat;
		qint64 currentEpoch = 0;
		LogCenterPrivate() {
			LogFileNameTimeFormat = VIApp->getEnvConfig(VIApplication::LogFileNameTimeFormat).toString();
			LogTimeFormat = VIApp->getEnvConfig(VIApplication::LogTimeFormat).toString();
			QString birthTime = QDateTime::currentDateTime().toString(LogFileNameTimeFormat);
			QString LogFolderPath = VIApp->getEnvConfig(VIApplication::LogFolderPath).toString();
			LogFile.setFileName(LogFolderPath % "/" % birthTime % ".log");
			QDir logDir(LogFolderPath);
			if (!logDir.exists()) {
				logDir.mkpath(".");
			}
			if (!LogFile.open(QIODevice::NewOnly | QIODevice::Text)) {
				const QString openFailMessage = QStringLiteral(
					"LogCenter: failed to open log file \"%1\" (it may already exist). File logging is disabled.")
					.arg(LogFile.fileName());
				qt_message_output(QtMsgType::QtWarningMsg, QMessageLogContext(), openFailMessage);
				return;
			}
			Stream = new QTextStream(&LogFile);
			Stream->setEncoding(QStringConverter::Utf8);
			Timer.setInterval(10000);
			QObject::connect(&Timer, &QTimer::timeout, [this]() {
				if (Pause) {
					return;
				}
				qint64 nowEpoch = QDateTime::currentMSecsSinceEpoch();
				if (nowEpoch - currentEpoch >= 10000) {
					save();
					Pause = true;
				}
				});
			Timer.start();
		}
		void log(QString line) {
			Pause = false;
			if (Stream) {
				*Stream << line << "\n";
				current++;
				if (current == cache) {
					current = 0;
					Stream->flush();
				}
			}
		}
		void save() {
			if (Stream != nullptr) {
				Stream->flush();
			}
		}
	};
	LogCenter* LogCenterPrivate::Instance = nullptr;

	/*!
		\class Visindigo::General::LogCenter
		\inheaderfile General/LogCenter.h
		\brief LogCenter是日志记录器管理器，用于管理全局日志对象并处理日志消息.
		\inmodule Visindigo
		\ingroup VIDebug
		\since Visindigo 0.13.0

		此类是VDebug的核心类，用于管理Logger实例并处理日志消息的输出。

		在Visindigo开发中，大部分情况下可以直接取代QDebug使用。如果直接比较
		\code
		vgDebug << "log"
		qDebug() << "log"
		\endcode
		则在VDebug确实需要QDebug约120%~200%的耗时，但这么比较并不公平，因为VDebug提供了更多的功能，
		例如日志级别、日志文件输出、日志重定向等，这些功能的实现需要额外的处理和资源。如果将VDebug与
		经过了相当设置的QDebug相比（即使用Qt的日志格式、输出重定向等功能之后），则二者耗时基本相当。

		VDebug和QDebug在结构上略有不同：
		\list
		\li QDebug使用QDebug类来承接日志消息，而VDebug则是LoggerMsgHandler类。
		\li QDebug实例可以使用QMessageLogger类获取，而VDebug则是通过Logger类获取。
		\li QMessageLogger使用不同的QMessageLoggingCategory构造QDebug来区分不同的日志命名空间，
		而VDebug则是通过Logger类的实例来区分不同的日志命名空间。
		\endlist
	*/

	/*!
		\since Visindigo 0.13.0
		构造LogCenter实例。
		\a threshold 全局日志级别阈值，低于此级别的日志将不会被输出。

		这是一个私有构造函数，用户无法直接调用此函数创建LogCenter实例。
		要获取LogCenter实例，请使用静态函数LogCenter::getInstance()。
		默认阈值为Logger::Debug，即默认输出所有级别（含调试信息），Debug与Release构建行为一致。
	*/
	LogCenter::LogCenter(Logger::Level threshold) {
		if (LogCenterPrivate::Instance != nullptr) {
			d = LogCenterPrivate::Instance->d;
			LogCenterPrivate::Instance->d = nullptr;
			delete LogCenterPrivate::Instance;
			LogCenterPrivate::Instance = this;
		}
		else {
			d = new LogCenterPrivate;
			LogCenterPrivate::Instance = this;
		}
		LogCenterPrivate::Instance->setGlobalLogLevel(threshold);
	}

	/*!
		\since Visindigo 0.13.0
		return LogCenter单例对象的指针。

		这是Visindigo单例类getInstance()函数的一个特例。其他单例类的getInstance()函数均会在实例不存在时创建它们。请注意区分。
	*/
	LogCenter* LogCenter::getInstance() {
		if (LogCenterPrivate::Instance == nullptr) {
			return new LogCenter();
		}
		else {
			return LogCenterPrivate::Instance;
		}
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	LogCenter::~LogCenter() {
		delete d;
	}

	QString LogCenter::getFormattedLogStr(LoggerMsgHandler* handler) {
		QString result = (handler->getMetaData().isValid() ? QString("[") % handler->getMetaData().toString() % QString("]: ") : QString(""))
			% handler->getMessage();
		return result;
	}

	QString LogCenter::getPlainLogStr(LoggerMsgHandler* handler) {
		QString result = (handler->getMetaData().isValid() ? QString("[") % handler->getMetaData().toString() % QString("]: ") : QString(""))
			% handler->getMessage();
		return result;
	}

	QString LogCenter::getStacktraceStr(LoggerMsgHandler* handler) {
		QString result;
		if (not handler->getStacktrace().isEmpty()) {
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
		return result;
	}
	/*!
		\since Visindigo 0.13.0
		处理日志消息。
		\a handler 指向LoggerMsgHandler对象的指针，包含了日志消息的相关信息。

		此函数由LoggerMsgHandler对象在其析构函数中自动调用，用于将日志消息输出到控制台和日志文件中。

		强烈不推荐用户手动调用此函数。
	*/
	void LogCenter::msgHandlerLog(LoggerMsgHandler* handler) {
		Logger::Level handlerLevel = handler->getLevel();
		Logger* logger = handler->getLogger();
		if (logger != nullptr && (int)handlerLevel < (int)logger->getThreshold()) {
			return;
		}
		if ((int)handlerLevel < (int)d->threshold) {
			return;
		}
		d->currentEpoch = QDateTime::currentMSecsSinceEpoch();
		QString timeStr = QString("[%1]").arg(QDateTime::fromMSecsSinceEpoch(d->currentEpoch).toString(d->LogTimeFormat));
		QString consoleStr; // 本条日志的控制台（可能含 ANSI 样式）整行，同时作为 logReceived 的 consoleStr 参数
		switch (handler->getLevel()) {
		case Logger::Level::Debug:
			consoleStr = timeStr % QStringLiteral("[Debug]   (") % handler->getLogger()->getNamespace() %
				QStringLiteral(") ") % getFormattedLogStr(handler);
			qt_message_output(QtMsgType::QtDebugMsg, QMessageLogContext(), consoleStr);
			d->log(timeStr % QStringLiteral("[Debug]   (") % handler->getLogger()->getNamespace() %
				QStringLiteral(") ") % getPlainLogStr(handler));
			break;
		case Logger::Level::Message:
			consoleStr = timeStr % QStringLiteral("[Message] (") % handler->getLogger()->getNamespace() %
				QStringLiteral(") ") % getFormattedLogStr(handler);
			qt_message_output(QtMsgType::QtInfoMsg, QMessageLogContext(), consoleStr);
			d->log(timeStr % QStringLiteral("[Message] (") % handler->getLogger()->getNamespace() %
				QStringLiteral(") ") % getPlainLogStr(handler));
			break;
		case Logger::Level::Notice:
			consoleStr = timeStr % Visindigo::Utility::Console::inNoticeStyle(QStringLiteral("[NOTICE]  (") %
				handler->getLogger()->getNamespace() % QStringLiteral(") ") % getFormattedLogStr(handler));
			qt_message_output(QtMsgType::QtInfoMsg, QMessageLogContext(), consoleStr);
			d->log(timeStr % QStringLiteral("[NOTICE]  (") % handler->getLogger()->getNamespace() %
				QStringLiteral(") ") % getPlainLogStr(handler));
			break;
		case Logger::Level::Success:
			consoleStr = timeStr % Visindigo::Utility::Console::inSuccessStyle(QStringLiteral("[Success] (") %
				handler->getLogger()->getNamespace() % QStringLiteral(") ") % getFormattedLogStr(handler));
			qt_message_output(QtMsgType::QtInfoMsg, QMessageLogContext(), consoleStr);
			d->log(timeStr % QStringLiteral("[Success] (") % handler->getLogger()->getNamespace() %
				QStringLiteral(") ") % getPlainLogStr(handler));
			break;
		case Logger::Level::Warning:
			consoleStr = timeStr % Visindigo::Utility::Console::inWarningStyle(QStringLiteral("[WARNING] (") %
				handler->getLogger()->getNamespace() % QStringLiteral(") ") % getFormattedLogStr(handler) % getStacktraceStr(handler));
			qt_message_output(QtMsgType::QtWarningMsg, QMessageLogContext(), consoleStr);
			d->log(timeStr % QStringLiteral("[WARNING] (") % handler->getLogger()->getNamespace() %
				QStringLiteral(") ") % getPlainLogStr(handler) % getStacktraceStr(handler));
			break;
		case Logger::Level::Error:
			consoleStr = timeStr % Visindigo::Utility::Console::inErrorStyle(QStringLiteral("[ERROR]   (") %
				handler->getLogger()->getNamespace() % QStringLiteral(") ") % getFormattedLogStr(handler) % getStacktraceStr(handler));
			qt_message_output(QtMsgType::QtCriticalMsg, QMessageLogContext(), consoleStr);
			d->log(timeStr % QStringLiteral("[ERROR]   (") % handler->getLogger()->getNamespace() %
				QStringLiteral(") ") % getPlainLogStr(handler) % getStacktraceStr(handler));
			break;
		}
		emit logReceived(handler->getLogger()->getNamespace(), handler->getLevel(), handler->getMessage(), consoleStr, handler->getMetaData());
	}

	/*!
		\since Visindigo 0.13.0
		设置全局日志级别阈值。
		\a level 全局日志级别阈值，低于此级别的日志将不会被输出。

		此函数允许用户动态调整全局日志级别阈值。
	*/
	void LogCenter::setGlobalLogLevel(Logger::Level level) {
		d->threshold = level;
	}

	/*!
		\since Visindigo 0.13.0
		将日志缓冲区的内容保存到日志文件中。

		此函数强制将日志缓冲区的内容写入日志文件中。
		通常情况下，日志会自动定期保存，但用户也可以手动调用此函数以确保日志被保存。
	*/
	void LogCenter::finalSave() {
		d->save();
	}

	/*!
		\since Visindigo 0.13.0
		生成崩溃报告。
		\a ex 指向Exception对象的引用，包含了崩溃的相关信息。
		此函数用于生成崩溃报告文件，包含了崩溃时的堆栈信息和其他相关数据。
		这个函数由VIApplication在捕获到会导致程序崩溃的异常时自动调用。

		这个函数在所有平台上可用，但只能处理Visindigo::General::Exception类型的异常，以及
		已经被充分转换为该类型的std异常。
	*/
	void LogCenter::generateCrashReport(const Exception& ex) {
		QString crashReportFolderPath = VIApp->getEnvConfig(VIApplication::LogFolderPath).toString() + "/crashreports";
		QDir crashReportDir(crashReportFolderPath);
		if (not crashReportDir.exists()) {
			crashReportDir.mkpath(".");
		}
		QString crashReportFileName = QDateTime::currentDateTime().toString(d->LogFileNameTimeFormat) % "_crashreport.log";
		QFile crashReportFile(crashReportFolderPath % "/" % crashReportFileName);
		if (crashReportFile.open(QIODevice::NewOnly | QIODevice::Text)) {
			QTextStream stream(&crashReportFile);
			stream.setEncoding(QStringConverter::Utf8);
			stream << "Visindigo Crash Report\n";
			stream << "Generated on " % QDateTime::currentDateTime().toString(Qt::ISODate) % "\n\n";
			stream << "When executing function:\n";
			stream << ex.getFunction() % " (" % ex.getFile() % ":" % QString::number(ex.getLine()) % ")\n\n";
			stream << "The following exception was thrown:\n";
			stream << "Exception Type: " % Exception::typeToString(ex.getType()) % "\n";
			stream << "Exception Message: " % ex.getMessage() % "\n\n";
			stream << "Stacktrace:\n";
			stream << "Index\tBinary File ! Function (+Address) in Source File at Line Number\n";
			quint32 index = 0;
			for (const StacktraceFrame& frame : ex.getStacktrace()) {
				stream << QString("%1\t%2 ! %3 (+%4) in %5 at %6\n")
					.arg(QString::number(index))
					.arg(frame.getBinaryFileName())
					.arg(frame.getFunctionName())
					.arg(QString::number(frame.getAddress(), 16).toUpper())
					.arg(frame.getSourceFileName())
					.arg(QString::number(frame.getLineNumber()));
				index++;
			}
			stream << "=========================================\n";
			stream << generateHardwareInfo(false, true) << "\n";
			crashReportFile.close();
		}
	}

	static inline bool isValidInterface(const QNetworkInterface& interface) {
		auto flags = interface.flags();
		if (not flags.testFlag(QNetworkInterface::IsUp)) return false;
		if (flags.testFlag(QNetworkInterface::IsLoopBack)) return false;
		QString mac = interface.hardwareAddress();
		if (mac.isEmpty() || mac == "00:00:00:00:00:00") return false;
		return true;
	}

	/*!
		\since Visindigo 0.15.2
		\a debugOutput 是否将生成的硬件信息报告输出到调试控制台。
		\a containsNetworkInfo 是否包含网络接口信息。

		return 生成硬件信息报告。
		
		如果 \a debugOutput 为true，则在返回相关信息的同时
		也会将报告输出到调试控制台。报告内容包括操作系统版本、内核版本、CPU架构、网络接口信息等。
		
		这个函数默认在程序启动，以及崩溃的时候被自动调用，以生成硬件信息报告。
		用户也可以手动调用此函数以获取当前的硬件信息。

		\warning 如果启用 \a containsNetworkInfo 选项，则报告中将包含网络接口的详细信息，
		这可能会暴露用户的隐私信息（如MAC地址、IP地址等）。请谨慎使用此选项。
		在生成崩溃日志时，会启用 \a containsNetworkInfo 选项以提供更全面的系统信息，但在其他情况下默认不启用以保护用户隐私。
	*/
	QString LogCenter::generateHardwareInfo(bool debugOutput, bool containsNetworkInfo) {
		Logger logger("HardwareInfoReport");
		QStringList content;
		content << "Operating System: " % QSysInfo::productType();
		content << "Pretty Product Name: " % QSysInfo::prettyProductName();
		content << "Product Version: " % QSysInfo::productVersion();
		content << "Kernel Type: " % QSysInfo::kernelType();
		content << "Kernel Version: " % QSysInfo::kernelVersion();
		content << "CPU Architecture: " % QSysInfo::currentCpuArchitecture();
		content << "Machine UUID: " % QString(QSysInfo::machineUniqueId().toHex());
		content << "Machine HostName: " % QSysInfo::machineHostName();
		if (containsNetworkInfo) {
			content << "Network Interfaces:";
			content << "----------------------------------------";
			QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
			for (const QNetworkInterface& interface : interfaces) {
				if (not isValidInterface(interface)) {
					continue;
				}
				content << "Interface Name: " % interface.name();
				content << "Human Readable Name: " % interface.humanReadableName();
				content << "MAC Address: " % interface.hardwareAddress();
				content << "Interface Type: " % QString(QMetaEnum::fromType<QNetworkInterface::InterfaceType>().valueToKey(interface.type()));
				QList<QNetworkAddressEntry> entries = interface.addressEntries();
				for (const QNetworkAddressEntry& entry : entries) {
					QHostAddress ip = entry.ip();
					if (ip.isLoopback()) continue;
					if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
						content << "  [IPv4] " % ip.toString();
					}
					else if (ip.protocol() == QAbstractSocket::IPv6Protocol) {
						content << "  [IPv6] " % ip.toString();
						if (!ip.scopeId().isEmpty()) {
							content << "    (Scope ID:" % ip.scopeId() % ")";
						}
					}
				}
				content << "----------------------------------------";
			}
		}
		QString report = content.join("\n");
		if (debugOutput) {
			logger.info() << report;
		}
		return report;
	}
}