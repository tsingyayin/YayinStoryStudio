#include "../LoggerMsgHandler.h"
#include "../LoggerManager.h"
#include <QtCore/qtimer.h>
namespace Visindigo::General {
	LoggerMsgHandler::LoggerMsgHandler(Logger* who, Logger::Level level) {
		d = new Visindigo::__Private__::LoggerMsgHandlerPrivate;
		d->Logger = who;
		d->Level = level;
	}
	LoggerMsgHandler::~LoggerMsgHandler() {
		LoggerManager::getInstance()->msgHandlerLog(this);
	}
	void LoggerMsgHandler::fromString(const QString& str) {
		d->Msg += str % " ";
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const QString& str) {
		fromString("\"" % str % "\"");
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(float num) {
		fromString(QString::number(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(double num) {
		fromString(QString::number(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(qint8 num) {
		fromString(QChar(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(quint8 num) {
		fromString(QString::number(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(qint16 num) {
		fromString(QString::number(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(quint16 num) {
		fromString(QString::number((quint32)num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(qint32 num) {
		fromString(QString::number(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(quint32 num) {
		fromString(QString::number(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(qint64 num) {
		fromString(QString::number(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(quint64 num) {
		fromString(QString::number(num));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(bool b) {
		fromString(b ? "true" : "false");
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const char* str) {
		fromString(QString::fromUtf8(str));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const QStringList& strList) {
		fromString(QString("[%1]").arg(strList.join(", ")));
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QObject* pointer) {
		if (pointer == nullptr) {
			fromString("nullptr");
		}
		else {
			fromString(QString("%1(%2)").arg(pointer->metaObject()->className()).arg((quint64)(void*)pointer, 0, 16));
		}
		return *this;
	}
	QString LoggerMsgHandler::getMessage() {
		return d->Msg;
	}
	Logger* LoggerMsgHandler::getLogger() {
		return d->Logger;
	}
	Logger::Level LoggerMsgHandler::getLevel() {
		return d->Level;
	}
}