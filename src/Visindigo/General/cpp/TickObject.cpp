#include "General/TickObject.h"
#include "General/Log.h"
#include <QtCore/qmutex.h>
#include <QtCore/qmap.h>
#include <QtGui/qevent.h>
#include <chrono>
#include <QtCore/qset.h>
#include <QtCore/qmap.h>
#include <QtCore/qdatetime.h>

namespace Visindigo::General {
	class TickObjectPrivate {
		friend class TickObject;
		friend class TickLoop;
		friend class TickLoopPrivate;
	protected:
		TickObject::UpdateType updateType = TickObject::UpdateType::Update;
		double fixUpdateInterval_ms = -1.0; // miliseconds
		double fixUpdateInterval_ms_dynamic = -1.0;
		double accumulatedTime_ms = 0.0;
		TickLoop* currentLoop = nullptr;
		bool aboutToDelete = false;
	};

	/*!
		\class Visindigo::General::TickObject
		\brief TickObject提供了一个基于时间的更新机制，允许用户在每个Tick周期执行特定的逻辑。
		\inmodule Visindigo
		\since Visindigo 0.16.0

		TickObject是一个抽象类，用户可以通过继承它来创建自己的Tick对象，通过重写onUpdate或onFixUpdate
		方法来定义在每个Tick周期执行的逻辑。用户可以通过enable和disable方法来控制Tick对象的启用状态，
		并通过setFixUpdateInterval方法来设置FixUpdate的时间间隔。
		
		TickLoop会在每个Tick周期调用所有启用的TickObject的onUpdate或onFixUpdate方法，取决于UpdateType设置。
		也就是说，一个TickObject在一次执行之内，只能是Update或FixeUpdate中的一种，不能同时是两者。当然可以
		中途切换UpdateType设置，但通常不推荐这么做。

		TickLoop寄生在Qt事件循环中，因此刻循环执行频率就是Qt事件循环的频率，如果不频繁处理大量耗时任务，速率甚至可以高达上千次每秒。

		在FixUpdate模式下，TickLoop会根据设定的固定时间间隔（毫秒）来调用onFixUpdate方法。
		在大部分情况下（当应用负载较低时），TickLoop调用onFixUpdate的频率理应基本接近设置预期。
		
		但在某些情况下（如应用负载过高时），TickLoop可能无法按预期频率调用onFixUpdate方法。为此，TickLoop提供了
		两种超时策略，请参见Visindigo::General::TickLoop的说明。

		值得指出的是，TickObject的执行线程固定为执行enable方法时所在的线程，因此用户应当确保在正确的线程中启用TickObject。
		需要跨线程启动TickObject时，应使用Qt信号槽机制的跨线程连接来调用enable方法，或者直接在目标线程中创建并启用TickObject。

		\note 这类替代旧版Visindigo中的VIAbstractBehavior、VIBasicBehavior和VITimedBehavior，提供更简明清晰的设计。
	*/

	/*!
		\enum Visindigo::General::TickObject::UpdateType
		\since Visindigo 0.16.0
		\value Update 每个Tick周期调用onUpdate方法。
		\value FixUpdate 每个固定Tick周期调用onFixUpdate方法，频率由setFixUpdateInterval设置。
	*/

	/*!
		\enum Visindigo::General::TickObject::PresetInterval
		\since Visindigo 0.16.0
		预设的固定更新时间间隔的枚举，它不直接记录值，由内部实现转换为对应的毫秒数。
		
		\value FPS_8 125ms，部分FPS游戏主逻辑的经典帧率。
		\value FPS_10 100ms，游戏Minecraft的红石刻频率。
		\value FPS_15 66.6667ms，不常用。
		\value FPS_20 50ms，游戏Minecraft的主逻辑刻频率。
		\value FPS_24 41.6667ms，电影的经典帧率，即在人眼中形成动画的最低帧率。
		\value FPS_30 33.3333ms，现代显示器刷新率的（一般情况下）最低帧率。
		\value FPS_60 16.6667ms，现代显示器的常见刷新率。
		\value FPS_70 14.2857ms，现代显示器的常见刷新率。
		\value FPS_120 8.3333ms，一些高刷新率显示器的常见刷新率。
		\value FPS_144 6.9444ms，一些高刷新率显示器的常见刷新率。
		\value FPS_240 4.1667ms，一些高刷新率显示器的常见刷新率。
		\value FPS_360 2.7778ms，一些高刷新率显示器的常见刷新率。
		\value FPS_480 2.0833ms，不常用。
	*/

	/*!
		\since Visindigo 0.16.0

		构造函数
	*/
	TickObject::TickObject() {
		d = new TickObjectPrivate();
	}


	/*!
		\since Visindigo 0.16.0

		析构函数

		\warning 不要在任何场景delete一个启用状态的TickObject对象。要释放内存，
		请使用disableAndDelete方法。或先调用disable方法，直到isEnabled的返回值
		变为false后再delete对象。
	*/
	TickObject::~TickObject() {
		delete d;
	}

	/*!
		\since Visindigo 0.16.0
		\a elapsedTime_ms 当前Tick周期的时间增量，单位为毫秒。

		每个更新周期调用的函数，用户可以通过重写此函数来定义在每个Tick周期执行的逻辑。
	*/
	void TickObject::onUpdate(double elapsedTime_ms) {
	
	}

	/*!
		\since Visindigo 0.16.0
		\a elapsedTime_ms 当前Tick周期的时间增量，单位为毫秒。

		每个固定更新周期调用的函数，用户可以通过重写此函数来定义在每个固定Tick周期执行的逻辑。

		值得指出的是，在FixUpdate模式下，elapsedTime_ms参数的值可能会根据TickLoop的FixTickTimeoutPolicy设置而有所不同。

		你不能假定它一定等于你通过setFixUpdateInterval设置的固定时间间隔，如果你这么假定，则在某些情况下（如应用负载过高时）可能会导致逻辑错误。
		如果你利用它进行某些物理计算，则这种错误假定必然影响物理计算的准确性（譬如某些应用会出现非常
		经典的在不同帧率下摩擦效果/加速效果不一致的问题）。请参见Visindigo::General::TickLoop的说明以了解更多细节。
	*/
	void TickObject::onFixUpdate(double elapsedTime_ms) {

	}

	/*!
		\since Visindigo 0.16.0
		\a loop 可选的TickLoop对象，如果为nullptr，则默认为当前线程的TickLoop。

		启用TickObject，挂载到目标TickLoop上。如果你不设置loop参数，则默认挂载到当前线程自己的TickLoop上。

		如果你需要挂载到其他线程的TickLoop上，请在目标线程中调用enable方法。

		如果你要挂载到自己创建的TickLoop上，请直接传入该TickLoop的指针。

		调用此函数后，isEnable会开始返回true，但它并不会立即参与到当前的事件循环中，只从下一次循环开始生效。
	*/
	void TickObject::enableUpdate(TickLoop* loop) {
		if (not d->currentLoop) {
			if (loop) {
				d->currentLoop = loop;
			}
			else {
				d->currentLoop = TickLoop::getThreadInstance();
			}
			d->currentLoop->enableTickObject(this);
		}
	}

	/*!
		\since Visindigo 0.16.0

		禁用TickObject，它会从当前TickLoop上卸载，使其停止接收更新调用。

		它会在所在的TickLoop的当前循环结束后才真正停止调用，因此即使调用disable，也可能
		在短时间内继续接收到更新调用，直到所在的TickLoop完成当前循环为止。

		你不能在调用disable后立即重新调用enable，因此此时它仍处于激活状态，重新enable不产生任何效果。

		你也不能在调用disble后立即销毁对象，要么手动判断在isEnabled为false时销毁，要么
		改用disableAndDelete方法。
	*/
	void TickObject::disableUpdate() {
		if (d->currentLoop) {
			d->currentLoop->disableTickObject(this);
		}
	}

	/*!
		\since Visindigo 0.16.0
		禁用并删除TickObject。它会先调用disable方法来禁用对象，然后在所在的TickLoop的当前循环结束后才真正删除对象。

		如果它当前未启用，则会直接删除对象。
	*/
	void TickObject::disableAndDelete() {
		if (d->currentLoop) {
			d->aboutToDelete = true;
			d->currentLoop->disableTickObject(this);
		}
		else {
			QObject* self = dynamic_cast<QObject*>(this);
			if (self) {
				self->deleteLater();
			}
			else {
				delete this;
			}
		}
	}

	/*!
		\since Visindigo 0.16.0
		return 是否启用状态。
	*/
	bool TickObject::isEnabled() const {
		return d->currentLoop != nullptr;
	}

	/*!
		\since Visindigo 0.16.0
		\a interval 固定更新时间间隔，单位为毫秒。

		设置FixUpdate的时间间隔。
	*/
	void TickObject::setFixUpdateInterval(double interval) {
		d->fixUpdateInterval_ms = interval;
		d->fixUpdateInterval_ms_dynamic = interval;
	}

	/*!
		\since Visindigo 0.16.0
		\a preset 预设的固定更新时间间隔。
		设置FixUpdate的时间间隔，使用预设的固定更新时间间隔。
	*/
	void TickObject::setFixUpdateInterval(PresetInterval preset) {
		static const QMap<PresetInterval, double> presetToInterval = {
			{PresetInterval::FPS_8, 125.0},
			{PresetInterval::FPS_10, 100.0},
			{PresetInterval::FPS_15, 66.6667},
			{PresetInterval::FPS_20, 50.0},
			{PresetInterval::FPS_24, 41.6667},
			{PresetInterval::FPS_30, 33.3333},
			{PresetInterval::FPS_60, 16.6667},
			{PresetInterval::FPS_70, 14.2857},
			{PresetInterval::FPS_120, 8.3333},
			{PresetInterval::FPS_144, 6.9444},
			{PresetInterval::FPS_240, 4.1667},
			{PresetInterval::FPS_360, 2.7778},
			{PresetInterval::FPS_480, 2.0833}
		};
		if (presetToInterval.contains(preset)) {
			setFixUpdateInterval(presetToInterval[preset]);
		}
	}
	/*!
		\since Visindigo 0.16.0

		return FixUpdate的时间间隔，单位为毫秒。
	*/
	double TickObject::getFixUpdateInterval() const {
		return d->fixUpdateInterval_ms;
	}

	/*!
		\since Visindigo 0.16.0
		return 当前时间戳，单位为毫秒。

		如果TickObject处于启用状态，则返回所在的TickLoop的当前时间戳；否则返回系统当前时间戳。

		请参见Visindigo::General::TickLoop::getCurrentMillisecondsSinceEpoch()以了解TickLoop的当前时间戳的定义和使用细节。
	*/
	qint64 TickObject::getCurrentMillisecondsSinceEpoch() const {
		return d->currentLoop ? d->currentLoop->getCurrentMillisecondsSinceEpoch() : QDateTime::currentMSecsSinceEpoch();
	}

	/*!
		\since Visindigo 0.16.0
		\a type 更新类型。

		设置更新类型，可以是Update或FixUpdate。
	*/
	void TickObject::setUpdateType(UpdateType type) {
		d->updateType = type;
		if (type == UpdateType::Update) {
			d->accumulatedTime_ms = 0.0;
		}
	}

	/*!
		\since Visindigo 0.16.0

		return 更新类型。
	*/
	TickObject::UpdateType TickObject::getUpdateType() const {
		return d->updateType;
	}


		

	class TickLoopPrivate {
		friend class TickLoop;
	protected:
		static QMap<Qt::HANDLE, TickLoop*> threadTickLoops;
		static QMutex threadTickLoopsMutex;
		static const QEvent::Type TickEventType = static_cast<QEvent::Type>(QEvent::User + 0x325);
		QSet<TickObject*> tickObjects;
		QList<qint32> fixUpdateIntervals;
		QThread* thread = nullptr;
		QMutex autoSteppingMutex;
		bool autoStepping = true;
		qint64 lastTickTime_ns = 0;
		TickLoop::FixTickTimeoutPolicy fixTickTimeoutPolicy = TickLoop::FixTickTimeoutPolicy::RealTime;
		QMutex manualStepMutex;
		double manualStepElapsedTime_ns = -1.0;
		QList<TickObject*> pendingEnableObjects;
		QMutex tickObjectsEnabledMutex;
		QList<TickObject*> pendingDisableObjects;
		QMutex tickObjectsDisabledMutex;
		qint64 currentMillisecondsSinceEpoch = 0;
		void stepTick(double elapsedTime_ns = -1.0) {
			QList<TickObject*> pendingEnable;
			tickObjectsEnabledMutex.lock();
			pendingEnable = pendingEnableObjects;
			pendingEnableObjects.clear();
			tickObjectsEnabledMutex.unlock();

			for (TickObject* obj : pendingEnable) {
				if (not tickObjects.contains(obj)) {
					tickObjects.insert(obj);
				}
			}
			qint64 currentTime_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
			currentMillisecondsSinceEpoch = currentTime_ns / 1'000'000;
			if (elapsedTime_ns < 0.0) {
				elapsedTime_ns = currentTime_ns - lastTickTime_ns;
			}
			lastTickTime_ns = currentTime_ns;
			double elapsedTime_ms = elapsedTime_ns / 1'000'000.0; // convert to miliseconds
			for (TickObject* obj : tickObjects) {
				if (obj->d->updateType == TickObject::UpdateType::Update) {
					obj->onUpdate(elapsedTime_ms);
				}
				else if (obj->d->updateType == TickObject::UpdateType::FixUpdate) {
					if (obj->d->fixUpdateInterval_ms > 0.0) {
						obj->d->accumulatedTime_ms += elapsedTime_ms;
						if (fixTickTimeoutPolicy == TickLoop::FixTickTimeoutPolicy::RealTime) {
							if (obj->d->accumulatedTime_ms >= obj->d->fixUpdateInterval_ms_dynamic) {
								obj->onFixUpdate(obj->d->accumulatedTime_ms);
								double delta = obj->d->accumulatedTime_ms - obj->d->fixUpdateInterval_ms;
								if (delta > obj->d->fixUpdateInterval_ms) {
									obj->d->fixUpdateInterval_ms_dynamic = obj->d->fixUpdateInterval_ms; // 超时量过大，直接重置为设定的固定时间间隔
								}
								else if (delta > 0.0 && delta < obj->d->fixUpdateInterval_ms) {
									obj->d->fixUpdateInterval_ms_dynamic = obj->d->fixUpdateInterval_ms - delta; // 弥补超时量
								}
								else {
									obj->d->fixUpdateInterval_ms_dynamic = obj->d->fixUpdateInterval_ms + delta; // 预留超时量
								}
								if (obj->d->fixUpdateInterval_ms_dynamic < 0.0) {
									obj->d->fixUpdateInterval_ms_dynamic = obj->d->fixUpdateInterval_ms; // 预留超时量过大，重置为设定的固定时间间隔
								}
								obj->d->accumulatedTime_ms = 0.0;
							}
						}
						else if (fixTickTimeoutPolicy == TickLoop::FixTickTimeoutPolicy::FixInterval) {
							while (obj->d->accumulatedTime_ms >= obj->d->fixUpdateInterval_ms) {
								obj->onFixUpdate(obj->d->fixUpdateInterval_ms);
								obj->d->accumulatedTime_ms -= obj->d->fixUpdateInterval_ms;
							}
						}
					}
					else {
						obj->onFixUpdate(elapsedTime_ms);
					}
				}
			}

			QList<TickObject*> pendingDisable;
			tickObjectsDisabledMutex.lock();
			pendingDisable = pendingDisableObjects;
			pendingDisableObjects.clear();
			tickObjectsDisabledMutex.unlock();

			for (TickObject* obj : pendingDisable) {
				if (tickObjects.contains(obj)) {
					tickObjects.remove(obj);
					obj->d->currentLoop = nullptr;
					if (obj->d->aboutToDelete) {
						QObject* qobj = dynamic_cast<QObject*>(obj);
						if (qobj) {
							qobj->deleteLater();
						}
						else {
							delete obj;
						}
					}
				}
			}
		}
	};
	
	QMap<Qt::HANDLE, TickLoop*> TickLoopPrivate::threadTickLoops;
	QMutex TickLoopPrivate::threadTickLoopsMutex;

	/*!
		\class Visindigo::General::TickLoop
		\brief TickLoop是一个基于Qt事件循环的时间驱动器。
		\inmodule Visindigo
		\since Visindigo 0.16.0

		TickLoop是一个基于Qt事件循环的时间驱动器，它会在每个Tick周期调用所有启用的TickObject的onUpdate或onFixUpdate方法，取决于UpdateType设置。

		TickLoop寄生在Qt事件循环中，因此刻循环执行频率就是Qt事件循环的频率，如果不频繁处理大量耗时任务，速率甚至可以高达上千次每秒。
		值得指出的是，TickLoop不保证在每个Tick周期内调用所有启用的TickObject的更新方法的顺序，因此用户不应依赖于特定的调用顺序来实现逻辑。

		在FixUpdate模式下，TickLoop会根据设定的固定时间间隔（毫秒）来调用onFixUpdate方法。
		在大部分情况下（当应用负载较低时），TickLoop调用onFixUpdate的频率理应基本接近设置预期。
		但在某些情况下（如应用负载过高时），TickLoop可能无法按预期频率调用onFixUpdate方法。为此，TickLoop提供了
		两种超时策略。

		\section1 RealTime 超时策略
		当FixTickTimeoutPolicy设置为RealTime时，TickLoop在每次调用onFixUpdate时都会传入实际的时间增量，而不是设定的固定时间间隔。
		这意味着如果应用负载过高导致TickLoop无法按预期频率调用onFixUpdate方法，elapsedTime_ms参数的值会相应增加，以反映实际的时间增量。
		它不追求按需要的次数去调用onFixUpdate，而追求每次调用时忠实反应时间的流逝。

		这种策略适用于需要根据实际时间增量进行物理计算的场景，可以在一定程度上保持物理计算的准确性。大部分情况下使用RealTime
		超时策略即可，这也是默认设置。

		\section1 FixInterval 超时策略
		当FixTickTimeoutPolicy设置为FixInterval时，TickLoop在每次调用onFixUpdate时永远传入固定的时间间隔，且按时间间隔的倍数去调用onFixUpdate方法。
		这在忠实进行每一次计算远比保持物理计算的准确性更重要的场景中非常有用。但这可能会导致灾难性的性能问题，因为如果应用负载过高，
		TickLoop可能会试图在短时间内调用大量的onFixUpdate方法来弥补未按预期频率调用的情况，从而进一步增加应用负载，形成恶性循环。

		除非你非常清楚你的应用场景需要FixInterval超时策略，否则强烈建议使用默认的RealTime超时策略。

		\section1 线程默认TickLoop与自己的TickLoop
		当通过getThreadInstance方法获取TickLoop实例时，如果目标线程尚未创建默认TickLoop实例，
		则会为其创建一个新的实例。也就是说，每个线程都有一个默认的TickLoop实例。大部分情况下，这就已经满足使用要求。

		但如果你需要在一个线程中使用多个TickLoop实例，则在该线程中直接创建新的TickLoop实例即可。
		每个TickLoop实例都可以独立管理自己的TickObject列表，并且它们之间不会互相干扰。

		\section1 注意事项
		\list 
		\li TickLoop寄生在Qt事件循环中，因此它的更新频率取决于Qt事件循环的频率。
		如果你在TickObject的更新方法中执行了大量耗时任务，可能会导致更新频率下降，从而影响应用的响应性。
		请确保在更新方法中避免执行过于耗时的操作，或者将耗时操作分散到多个Tick周期中。
		\li 你可以在onUpdate和onFixUpdate中安全的调用对应线程的exec()函数进入新的循环层级，这不会发生递归调用，
		因为TickLoop的事件处理函数会在每次循环结束时才调用更新方法。在exec()进入新的循环层级后，TickLoop会被
		冻结在当前层级，直到新的循环层级退出后才会继续调用更新方法。

	*/

	/*!
		\since Visindigo 0.16.0
		\enum Visindigo::General::TickLoop::FixTickTimeoutPolicy
		\value RealTime 实际时间超时策略
		\value FixInterval 固定间隔调用策略
	*/

	/*!
		\since Visindigo 0.16.0
		\a targetThread 目标线程，如果为nullptr，则默认为当前线程。

		获取目标线程默认的TickLoop实例，如果目标线程尚未创建默认TickLoop实例，则会为其创建一个新的实例。
	*/
	TickLoop* TickLoop::getThreadInstance(QThread* targetThread) {
		Qt::HANDLE threadId = (targetThread ? targetThread->currentThreadId() : QThread::currentThreadId());
		QMutexLocker locker(&TickLoopPrivate::threadTickLoopsMutex);
		if (TickLoopPrivate::threadTickLoops.contains(threadId)) {
			return TickLoopPrivate::threadTickLoops[threadId];
		}
		else {
			targetThread = targetThread ? targetThread : QThread::currentThread();
			TickLoop* newLoop = new TickLoop(targetThread);
			TickLoopPrivate::threadTickLoops[threadId] = newLoop;
			return newLoop;
		}
	}

	/*!
		\since Visindigo 0.16.0
		\a targetThread 目标线程。

		构造函数。
		构造后默认开始循环。
	*/
	TickLoop::TickLoop(QThread* targetThread) {
		vgNotice << "Creating TickLoop";
		d = new TickLoopPrivate();
		if (targetThread == nullptr) {
			targetThread = QThread::currentThread();
		}
		d->thread = targetThread;
		this->moveToThread(targetThread);
		
		if (d->autoStepping) {
			d->lastTickTime_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
			qApp->postEvent(this, new QEvent(TickLoopPrivate::TickEventType));
		}
		vgSuccess << "TickLoop created in thread" << targetThread;
	}

	/*!
		\since Visindigo 0.16.0
		析构函数。
	*/
	TickLoop::~TickLoop() {
		QMutexLocker locker(&TickLoopPrivate::threadTickLoopsMutex);
		Qt::HANDLE threadId = d->thread->currentThreadId();
		if (TickLoopPrivate::threadTickLoops.contains(threadId) && TickLoopPrivate::threadTickLoops[threadId] == this) {
			TickLoopPrivate::threadTickLoops.remove(threadId);
		}
		QList<TickObject*> pendingEnable;
		d->tickObjectsEnabledMutex.lock();
		pendingEnable = d->pendingEnableObjects;
		d->pendingEnableObjects.clear();
		d->tickObjectsEnabledMutex.unlock();
		for (TickObject* obj : pendingEnable) {
			if (not d->tickObjects.contains(obj)) {
				d->tickObjects.insert(obj);
			}
		}
		QList<TickObject*> pendingDisable;
		d->tickObjectsDisabledMutex.lock();
		pendingDisable = d->pendingDisableObjects;
		d->pendingDisableObjects.clear();
		d->tickObjectsDisabledMutex.unlock();
		for (TickObject* obj : pendingDisable) {
			if (d->tickObjects.contains(obj)) {
				d->tickObjects.remove(obj);
				obj->d->currentLoop = nullptr;
				if (obj->d->aboutToDelete) {
					QObject* qobj = dynamic_cast<QObject*>(obj);
					if (qobj) {
						qobj->deleteLater();
					}
					else {
						delete obj;
					}
				}
			}
		}
		for (TickObject* obj : d->tickObjects) {
			obj->d->currentLoop = nullptr;
		}
		delete d;
	}

	/*!
		\since Visindigo 0.16.0
		\a event 事件对象。

		重写事件处理函数，用于TickLoop寄生到Qt事件循环中。如果你要在派生类
		中重写事件处理函数，请务必调用基类的实现以确保TickLoop的正常工作。
	*/
	bool TickLoop::event(QEvent* event) {
		if (event->type() == TickLoopPrivate::TickEventType) {
			if (d->autoStepping) {
				d->stepTick();
				qApp->postEvent(this, new QEvent(TickLoopPrivate::TickEventType));
			}
			else {
				d->stepTick(d->manualStepElapsedTime_ns);
			}
			return true;
		}
		return QObject::event(event);
	}

	/*!
		\since Visindigo 0.16.0
		\a obj 要启用的TickObject对象。

		 启用TickObject对象，使其开始接收TickLoop的更新调用。
		 如果对象已经启用，则此函数不会有任何效果。

		 请注意，启用一个TickObject会将其添加到TickLoop的更新列表中，因此在启用之前请确保你已经正确设置了TickObject的属性
		 （如UpdateType和FixUpdateInterval），以避免在启用后出现不符合预期的行为。

		这个函数是线程安全的，你可以在任何线程中调用它。
	*/
	void TickLoop::enableTickObject(TickObject* obj) {
		QMutexLocker locker(&d->tickObjectsEnabledMutex);
		if (not d->pendingEnableObjects.contains(obj)){
			obj->d->currentLoop = this;
			d->pendingEnableObjects.append(obj);
			vgInfo << "Enabling TickObject" << obj << "in TickLoop" << this;
		}
	}

	/*!
		\since Visindigo 0.16.0
		\a obj 要禁用的TickObject对象。

		 禁用TickObject对象，使其停止接收TickLoop的更新调用。
		 如果对象已经禁用，则此函数不会有任何效果。

		这个函数是线程安全的，你可以在任何线程中调用它。
	*/
	void TickLoop::disableTickObject(TickObject* obj) {
		if (not obj->d->currentLoop || obj->d->currentLoop != this) {
			return;
		}
		QMutexLocker locker(&d->tickObjectsDisabledMutex);
		if (not d->pendingDisableObjects.contains(obj)) {
			d->pendingDisableObjects.append(obj);
		}
	}

	/*!
		\since Visindigo 0.16.0
		\a autoStepping 是否自动步进。
		设置是否自动步进Tick事件，如果为true，则TickLoop会自动与Qt事件循环一起步进。

		如果为false，则需要手动调用stepTick方法来继续循环。
		默认值为true，通常不需要修改它。

		这个函数是线程安全的，你可以在任何线程中调用它。
	*/
	void TickLoop::setAutoStepping(bool autoStepping) {
		QMutexLocker locker(&d->autoSteppingMutex);
		if (d->autoStepping != autoStepping) {
			d->autoStepping = autoStepping;
			if (autoStepping) {
				d->lastTickTime_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
				qApp->postEvent(this, new QEvent(TickLoopPrivate::TickEventType));
			}
		}		
	}

	/*!
		\since Visindigo 0.16.0

		return 是否自动步进。

		这个函数是线程安全的，你可以在任何线程中调用它。
	*/
	bool TickLoop::isAutoStepping() const {
		QMutexLocker locker(&d->autoSteppingMutex);
		return d->autoStepping;
	}

	/*!
		\since Visindigo 0.16.0
		\a elapsedTime_ns 手动步进的时间增量，单位为纳秒。
		
		手动步进Tick事件，只有当自动步进被禁用时才有意义。调用此函数后，TickLoop会计划在
		下一个事件循环内，用此函数设置的手动步进时间增量进行相关调度。
		
		如果elapsedTime_ns参数为负值（默认即为负值），则TickLoop会自动计算自上次Tick事件以来的实际时间增量，并使用该增量进行调度。

		如果手动指定时间增量，则TickLoop会使用该增量进行调度，而不考虑实际时间的流逝。这在某些特殊场景下可能会有用。

		这个函数是线程安全的，你可以在任何线程中调用它。
	*/
	void TickLoop::stepTick(double elapsedTime_ns) {
		if (isAutoStepping()) {
			return;
		}
		QMutexLocker locker(&d->manualStepMutex);
		d->manualStepElapsedTime_ns = elapsedTime_ns;
		qApp->postEvent(this, new QEvent(TickLoopPrivate::TickEventType));
	}

	/*!
		\since Visindigo 0.16.0
		\a policy 超时策略。

		 设置FixUpdate的超时策略。
	*/
	void TickLoop::setFixTickTimeoutPolicy(FixTickTimeoutPolicy policy) {
		d->fixTickTimeoutPolicy = policy;
	}

	/*!
		\since Visindigo 0.16.0
		return 当前的毫秒级时间戳，单位为毫秒。

		注意，这个时间戳不是QDateTime::currentMSecsSinceEpoch的实时值，而是
		当前（或最后一次）Tick循环开始时的值，它在每次Tick循环开始时更新一次，周期内保持不变。

		这个函数被TickObject::getCurrentMillisecondsSinceEpoch调用，以提供给TickObject的更新方法使用。

		它存在的价值在于，获取系统时间戳实际实际上相当耗时，而大部分场景下又对时间戳的实时性要求并不高，
		整个Tick循环内所有TickObject要获取当前时间，实际上基本可以使用同一个值。
		因此提供了这个相对较为轻量的时间戳获取方式，以供TickObject的更新方法使用。
	*/
	qint64 TickLoop::getCurrentMillisecondsSinceEpoch() const {
		return d->currentMillisecondsSinceEpoch;
	}

	/*!
		\since Visindigo 0.16.0
		 return FixUpdate的超时策略。
	*/
	TickLoop::FixTickTimeoutPolicy TickLoop::getFixTickTimeoutPolicy() const {
		return d->fixTickTimeoutPolicy;
	}
}