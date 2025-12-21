#include "../Logger.h"
#include <QtCore/qstring.h>
#include "../LoggerMsgHandler.h"
namespace Visindigo::General {
	class LoggerPrivate {
		friend class Logger;
	protected:
		Logger::Level Threshold;
		QString Namespace;
		static Logger* DefaultLogger;
	};
	Logger* LoggerPrivate::DefaultLogger = nullptr;

	/*!
		\class Visindigo::General::Logger
		\inheaderfile General/Logger.h
		\brief 日志记录器类，用于记录和管理日志消息。
		\inmodule Visindigo
		\since Visindigo 0.13.0
		\ingroup VDebug
		
		Logger 类是日志记录器的实例组件，用于管理来自不同命名空间的日志消息。
		它允许设置日志级别阈值，以过滤不需要的日志消息。

		Logger 不是必须创建的，可以用global()函数获取全局记录器实例。
		但这等同于向带有Global命名空间的日志输出所有消息。

		如果需要更细粒度的日志管理，则应该创建自己的 Logger 实例。

		\note 此类不容许拷贝或移动。
	*/

	/*!
		\enum Visindigo::General::Logger::Level
		\since Visindigo 0.13.0
		\value Debug 调试级别日志消息，级别1000
		\value Message 普通消息级别日志消息，级别2000
		\value Info 信息级别日志消息，级别2000
		\value Log 日志级别日志消息，级别2000
		\value Notice 通知级别日志消息，级别3000
		\value Success 成功级别日志消息，级别4000
		\value Warning 警告级别日志消息，级别5000
		\value Error 错误级别日志消息，级别6000

		日志级别用于指示日志消息的重要性或严重性。较低的级别表示较少的重要性，而较高的级别表示更高的重要性。

		用户可以自定义一个整数作为阈值，然后将其强制类型转换为Level类型传递给setThreshold()函数，以设置日志记录器的日志级别阈值。
		只有级别高于或等于此阈值的日志消息才会被记录。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数，使用指定的命名空间和日志级别阈值创建一个日志记录器对象。

		\a nameSpace 日志记录器的命名空间。
		\a threshold 日志级别阈值。只有级别高于或等于此阈值的日志消息才会被记录。

		在QT_DEBUG模式下，默认阈值为Debug级别；否则默认为Message级别。

		这很重要，因为在发布版本中，调试级别的日志消息通常是不需要的。

		如果您在发行版本缺少日志输出，应考虑调整阈值。
	*/
	Logger::Logger(const QString& nameSpace, Level threshold) {
		d = new LoggerPrivate;
		d->Namespace = nameSpace;
		d->Threshold = threshold;
	}

	/*!
		\since Visindigo 0.13.0
		析构函数，销毁日志记录器对象。
	*/
	Logger::~Logger() {
		if (d != nullptr) {
			delete d;
		}
	}

	/*!
		\since Visindigo 0.13.0
		return 全局日志记录器实例。
	*/
	Logger* Logger::global() {
		if (LoggerPrivate::DefaultLogger == nullptr) {
			LoggerPrivate::DefaultLogger = new Logger("Global", Logger::Level::Debug);
		}
		return LoggerPrivate::DefaultLogger;
	}

	/*!
		\since Visindigo 0.13.0
		return 用于记录调试级别日志消息的日志消息处理器。
	*/
	LoggerMsgHandler Logger::debug() {
		return LoggerMsgHandler(this, Logger::Level::Debug);
	}

	/*!
		\since Visindigo 0.13.0
		return 用于记录普通消息级别日志消息的日志消息处理器。
	*/
	LoggerMsgHandler Logger::message() {
		return LoggerMsgHandler(this, Logger::Level::Message);
	}

	/*!
		\since Visindigo 0.13.0
		return 用于记录信息级别日志消息的日志消息处理器。
	*/
	LoggerMsgHandler Logger::info() {
		return LoggerMsgHandler(this, Logger::Level::Info);
	}

	/*!
		\since Visindigo 0.13.0
		return 用于记录日志级别日志消息的日志消息处理器。
	*/
	LoggerMsgHandler Logger::log() {
		return LoggerMsgHandler(this, Logger::Level::Log);
	}

	/*!
		\since Visindigo 0.13.0
		return 用于记录通知级别日志消息的日志消息处理器。
	*/
	LoggerMsgHandler Logger::notice() {
		return LoggerMsgHandler(this, Logger::Level::Notice);
	}

	/*!
		\since Visindigo 0.13.0
		return 用于记录成功级别日志消息的日志消息处理器。
	*/
	LoggerMsgHandler Logger::success() {
		return LoggerMsgHandler(this, Logger::Level::Success);
	}

	/*!
		\since Visindigo 0.13.0
		return 用于记录警告级别日志消息的日志消息处理器。
	*/
	LoggerMsgHandler Logger::warning() {
		return LoggerMsgHandler(this, Logger::Level::Warning);
	}

	/*!
		\since Visindigo 0.13.0
		return 用于记录错误级别日志消息的日志消息处理器。
	*/
	LoggerMsgHandler Logger::error() {
		return LoggerMsgHandler(this, Logger::Level::Error);
	}

	/*!
		\since Visindigo 0.13.0
		return 获取日志记录器的命名空间。
	*/
	QString Logger::getNamespace() {
		return d->Namespace;
	}

	/*!
		\since Visindigo 0.13.0
		\a nameSpace 日志记录器的命名空间。
		设置日志记录器的命名空间。
	*/
	void Logger::setNamespace(const QString& nameSpace) {
		d->Namespace = nameSpace;
	}

	/*!
		\since Visindigo 0.13.0
		return 获取日志级别阈值。
	*/
	Logger::Level Logger::getThreshold() const {
		return d->Threshold;
	}

	/*!
		\since Visindigo 0.13.0
		\a threshold 日志级别阈值。
		设置日志级别阈值。
	*/
	void Logger::setThreshold(Level threshold) {
		d->Threshold = threshold;
	}
}