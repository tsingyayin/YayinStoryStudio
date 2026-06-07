#ifndef Visindigo_Utility_BenchmarkTimer_h
#define Visindigo_Utility_BenchmarkTimer_h
#include "VICompileMacro.h"
#include <QtCore/qstring.h>

namespace Visindigo::Utility {
	class BenchmarkTimerPrivate;
	class VisindigoAPI BenchmarkTimer {
	public:
		BenchmarkTimer(bool autoStart = true, const QString& name = "BenchmarkTimer", bool autoLog = true);
		~BenchmarkTimer();
	public:
		void setLogCount(qint32 count);
		qint32 getLogCount() const;
		void start();
		double stop();
		double elapsed();
		double counter();
		double pause();
		double resume();
		double restart();
		double totalPaused();
		bool isRunning() const;
		bool isPaused() const;
	private:
		BenchmarkTimerPrivate* d;
	};
};
#endif // Visindigo_Utility_BenchmarkTimer_h
