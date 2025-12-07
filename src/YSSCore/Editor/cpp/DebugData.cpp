#include <QtCore/qstring.h>
#include "../DebugData.h"

namespace YSSCore::Editor {
	class DebugDataPrivate {
		friend class DebugData;
	protected:
		QString ExeName;
		quint64 MemUsage = 0;
		quint64 LiveTime = 0;
		bool Paused = false;
	};

	DebugData::DebugData() {
		d = new DebugDataPrivate;
	}

	DebugData::~DebugData() {
		delete d;
	}
	DebugData::DebugData(const DebugData& other) {
		d = new DebugDataPrivate;
		d->ExeName = other.d->ExeName;
		d->MemUsage = other.d->MemUsage;
		d->LiveTime = other.d->LiveTime;
		d->Paused = other.d->Paused;
	}

	DebugData::DebugData(DebugData&& other) {
		d = other.d;
		other.d = nullptr;
	}

	DebugData& DebugData::operator=(const DebugData& other) {
		if (this != &other) {
			d->ExeName = other.d->ExeName;
			d->MemUsage = other.d->MemUsage;
			d->LiveTime = other.d->LiveTime;
			d->Paused = other.d->Paused;
		}
		return *this;
	}

	DebugData& DebugData::operator=(DebugData&& other) {
		if (this != &other) {
			delete d;
			d = other.d;
			other.d = nullptr;
		}
		return *this;
	}

	void DebugData::setExecutableName(const QString& name) {
		d->ExeName = name;
	}

	void DebugData::setMemoryUsage(quint64 memUsage) {
		d->MemUsage = memUsage;
	}

	void DebugData::setLiveTime(quint64 liveTime) {
		d->LiveTime = liveTime;
	}

	void DebugData::setPaused(bool paused) {
		d->Paused = paused;
	}

	QString DebugData::getExecutableName() const {
		return d->ExeName;
	}

	quint64 DebugData::getMemoryUsage() const {
		return d->MemUsage;
	}

	quint64 DebugData::getLiveTime() const {
		return d->LiveTime;
	}

	bool DebugData::isPaused() const {
		return d->Paused;
	}
}