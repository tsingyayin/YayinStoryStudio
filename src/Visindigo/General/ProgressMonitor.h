#ifndef Visindigo_General_ProgressMonitor_h
#define Visindigo_General_ProgressMonitor_h
#include "VICompileMacro.h"
#include <QtCore/qobject.h>
namespace Visindigo::General {
	class ProgressMonitor;
	class ProgressHandlePrivate;
	class VisindigoAPI ProgressTracker {
		friend class ProgressMonitor;
	protected:
		ProgressTracker(qint32 id) {};
	public:
		VIMoveable(ProgressTracker);
		ProgressTracker(const ProgressTracker& other) = delete; // not copyable
		ProgressTracker& operator=(const ProgressTracker& other) = delete; // not copyable
		~ProgressTracker();
		qint32 getID() const;
		void update(float progress);
		void finish();
	private:
		ProgressHandlePrivate* d;
	};

	class ProgressMonitorPrivate;
	class VisindigoAPI ProgressMonitor :public QObject {
	public:
		enum MonitorType {
			Local, // monitor running in same process. UI may some times freeze, but VisindigoPeripheral.exe is not required.
			External // monitor running in different process. UI will not freeze, but VisindigoPeripheral.exe is required.
		};
	private:
		ProgressMonitor() {};
	public:
		static ProgressMonitor* getInstance();
		void setMonitorType(MonitorType type);
		void setExternalMonitorID(const QString& id);
		MonitorType getMonitorType() const;
		ProgressTracker startProgress(const QString& progressName, qint32 waitingTime = 5000, qint32 stepWaitingTime = 100);
	private:
		ProgressMonitorPrivate* d;
	};
}
#endif // Visindigo_General_ProgressMonitor_h