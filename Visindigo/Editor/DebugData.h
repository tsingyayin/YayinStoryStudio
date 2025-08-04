#pragma once
#include <QtCore/qtypes.h>
#include "../Macro.h"

class QString;
namespace Visindigo::Editor {
	class DebugDataPrivate;
	class VisindigoAPI DebugData {
	public:
		DebugData();
		~DebugData();
		DebugData(const DebugData& other);
		DebugData(DebugData&& other);
		DebugData& operator=(const DebugData& other);
		DebugData& operator=(DebugData&& other);
		void setExecutableName(const QString& name);
		void setMemoryUsage(quint64 memoryUsage);
		void setLiveTime(quint64 liveTime);
		void setPaused(bool paused);
		QString getExecutableName() const;
		quint64 getMemoryUsage() const;
		quint64 getLiveTime() const;
		bool isPaused() const;
	private:
		DebugDataPrivate* d;
	};
}