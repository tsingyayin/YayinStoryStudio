#include "../DebugFailedData.h"
#include <QtCore/qstring.h>

namespace YSSCore::Editor {
	class DebugFailedDataPrivate {
		friend class DebugFailedData;
	protected:
		QString IndexFileName;
		quint32 IndexFileLine = 0;
		quint32 IndexChar = 0;
		QString Title;
		QString Description;
		QStringList StackTrace;
	};
	DebugFailedData::DebugFailedData() {
		d = new DebugFailedDataPrivate;
	}
	DebugFailedData::~DebugFailedData() {
		delete d;
	}
	DebugFailedData::DebugFailedData(const DebugFailedData& other) {
		d = new DebugFailedDataPrivate;
		d->IndexFileName = other.d->IndexFileName;
		d->IndexFileLine = other.d->IndexFileLine;
		d->IndexChar = other.d->IndexChar;
		d->Title = other.d->Title;
		d->Description = other.d->Description;
		d->StackTrace = other.d->StackTrace;
	}
	DebugFailedData::DebugFailedData(DebugFailedData&& other) {
		d = other.d;
		other.d = nullptr;
	}
	DebugFailedData& DebugFailedData::operator=(const DebugFailedData& other) {
		if (this != &other) {
			d->IndexFileName = other.d->IndexFileName;
			d->IndexFileLine = other.d->IndexFileLine;
			d->IndexChar = other.d->IndexChar;
			d->Title = other.d->Title;
			d->Description = other.d->Description;
			d->StackTrace = other.d->StackTrace;
		}
		return *this;
	}
	DebugFailedData& DebugFailedData::operator=(DebugFailedData&& other) {
		if (this != &other) {
			delete d;
			d = other.d;
			other.d = nullptr;
		}
		return *this;
	}
	void DebugFailedData::setIndex(const QString& fileName, quint32 line, quint32 character) {
		d->IndexFileName = fileName;
		d->IndexFileLine = line;
		d->IndexChar = character;
	}
	void DebugFailedData::setTitle(const QString& title) {
		d->Title = title;
	}
	void DebugFailedData::setDescription(const QString& description) {
		d->Description = description;
	}
	void DebugFailedData::setStackTrace(const QStringList& stackTrace) {
		d->StackTrace = stackTrace;
	}
	QString DebugFailedData::getFileName() const {
		return d->IndexFileName;
	}
	quint32 DebugFailedData::getLine() const {
		return d->IndexFileLine;
	}
	quint32 DebugFailedData::getChar() const {
		return d->IndexChar;
	}
	QString DebugFailedData::getTitle() const {
		return d->Title;
	}
	QString DebugFailedData::getDescription() const {
		return d->Description;
	}
	QStringList DebugFailedData::getStackTrace() const {
		return d->StackTrace;
	}
}