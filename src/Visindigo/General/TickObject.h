#ifndef Visindigo_General_TickObject_h
#define Visindigo_General_TickObject_h
#include "VICompileMacro.h"
#include <QtCore/qobject.h>
#include <QtCore/qthread.h>
namespace Visindigo::General{
	class TickObjectPrivate;
	class TickLoopPrivate;
	class TickLoop;

	class VisindigoAPI TickObject {
		friend class TickLoop;
		friend class TickLoopPrivate;
	public:
		enum class UpdateType {
			Update,
			FixUpdate
		};
		enum class PresetInterval {
			FPS_8, FPS_10, FPS_15, FPS_20, FPS_24, FPS_30, 
			FPS_60, FPS_70, FPS_120, FPS_144, FPS_240,
			FPS_360, FPS_480
		};
	public:
		TickObject();
		virtual ~TickObject();
	public:
		virtual void onUpdate(double elapsedTime_ms); // miliseconds
		virtual void onFixUpdate(double elapsedTime_ms); // miliseconds
		void enableUpdate(TickLoop* loop = nullptr);
		void disableUpdate();
		void disableAndDelete();
		bool isEnabled() const;
		void setFixUpdateInterval(double interval);
		void setFixUpdateInterval(PresetInterval preset);
		double getFixUpdateInterval() const;
		qint64 getCurrentMillisecondsSinceEpoch() const;
		void setUpdateType(UpdateType type);
		UpdateType getUpdateType() const;
	protected:
		TickObjectPrivate* d;
	};
	
	class VisindigoAPI TickLoop :public QObject {
		Q_OBJECT;
	public:
		enum class FixTickTimeoutPolicy {
			RealTime,
			FixInterval
		};
	public:
		TickLoop(QThread* targetThread);
		virtual ~TickLoop();
		static TickLoop* getThreadInstance(QThread* targetThread = nullptr);
		void enableTickObject(TickObject* obj);
		void disableTickObject(TickObject* obj);
		void setAutoStepping(bool autoStepping);
		bool isAutoStepping() const;
		void stepTick(double elapsedTime_ns = -1.0); // nanoseconds
		void setFixTickTimeoutPolicy(FixTickTimeoutPolicy policy);
		qint64 getCurrentMillisecondsSinceEpoch() const;
		FixTickTimeoutPolicy getFixTickTimeoutPolicy() const;
	protected:
		bool event(QEvent* event) override;
	private:
		TickLoopPrivate* d;
	};
}
#endif // Visindigo_General_TickObject_h
