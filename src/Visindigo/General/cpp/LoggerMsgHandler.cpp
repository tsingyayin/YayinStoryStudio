#include "../LoggerMsgHandler.h"
#include "../LoggerManager.h"
#include <QtCore/qtimer.h>
#include <QtCore/qobject.h>
#include "../../Utility/Console.h"
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
	LoggerMsgHandler& LoggerMsgHandler::operator<<(const QByteArray& byteArray) {
		fromString("\n"+Visindigo::Utility::Console::binaryToString(byteArray));
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
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, QObject*> pointer_map) {
		QStringList list;
		for (auto it = pointer_map.begin(); it != pointer_map.end(); ++it) {
			if (it.value() == nullptr) {
				list.append(it.key() % ": nullptr");
			}
			else {
				list.append(it.key() % ": " % it.value()->metaObject()->className() % "(" % QString::number((quint64)(void*)it.value(), 16) % ")");
			}
		}
		fromString("{" % list.join(", ") % "}");
		return *this;
	}
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<QObject*> qobject_list) {
		QStringList list;
		for (QObject* obj : qobject_list) {
			if (obj == nullptr) {
				list.append("nullptr");
			}
			else {
				list.append(QString(obj->metaObject()->className()) % "(" % QString::number((quint64)(void*)obj, 16) % ")");
			}
		}
		fromString("[" % list.join(", ") % "]");
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