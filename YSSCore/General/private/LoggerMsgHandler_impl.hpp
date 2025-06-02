#pragma once

namespace YSSCore::General {
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(T t) {
		d->Msg += t.toString();
		return *this;
	}
	template<typename T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> pointer_map) {
		QString temp = "QMap{";
		for (QString key: pointer_map) {
			void* ptr = &pointer_map[key];
			temp += " \"" % key % "\": [Object at " % QString::number(ptr,16)%"],";
		}
		temp.removeLast();
		temp += "}";
		d->Msg += temp;
		return *this;
	}
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QMap<QString, T> pointer_map) {
		QString temp = "QMap{";
		for (QString key : pointer_map) {
			temp += " \"" % key % "\": " % pointer_map[key].toString() % ",";
		}
		temp.removeLast();
		temp += "}";
		d->Msg += temp;
		return *this;
	}
	template<typename T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> list) {
		QString temp = "QList[";
		for (int i = 0; i < list.size(); i++) {
			void* ptr = &list[i];
			temp += " [Object at " % QString::number(ptr, 16) % "],";
		}
		temp.removeLast();
		temp += "}";
		d->Msg += temp;
		return *this;
	}
	template<Printable T>
	LoggerMsgHandler& LoggerMsgHandler::operator<<(QList<T> list) {
		QString temp = "QList[";
		for (int i = 0; i < list.size(); i++) {
			temp += " " % list[i].toString() % ",";
		}
		temp.removeLast();
		temp += "}";
		d->Msg += temp;
		return *this;
	}
}