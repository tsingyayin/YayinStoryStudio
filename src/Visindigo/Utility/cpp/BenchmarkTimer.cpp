#include "Utility/BenchmarkTimer.h"
#include "General/Log.h"
#include <chrono>
namespace Visindigo::Utility {
	class BenchmarkTimerPrivate {
		friend BenchmarkTimer;
	private:
		qint32 logCount = 1;
		qint32 currentCount = 0;
		bool autoLog = true;
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> pausedTime;
		std::chrono::duration<double> totalPausedTime = std::chrono::duration<double>::zero();
		bool isRunning = false;
		bool isPaused = false;
		Visindigo::General::Logger* logger;
	};

	/*!
		\class Visindigo::Utility::BenchmarkTimer
		\brief 一个用于测量代码执行时间的计时器类，支持自动记录日志、暂停和恢复计时等功能。
		\since Visindigo 0.16.0
		\inmodule Visindigo

		BenchmarkTimer是一个实用的计时器类，设计用于测量代码块的执行时间。
		它提供了多种功能，包括自动记录日志、暂停和恢复计时、以及在达到指定计数时自动记录日志等。

		由于这个类在默认情况下自动记录日志，且在创建与销毁时自动开始与停止，因此它对于那些
		需要衡量代码执行时间且衡量区域恰好是一组代码块的情况非常方便，譬如你可以直接在函数的最开始
		新建一个BenchmarkTimer对象，然后什么都不用做，到函数结束时它会自动记录整个函数的执行时间。

		此外，这类也为循环体中测量每次迭代的执行时间提供了便利的counter()方法，
		配合setLogCount()方法可以在每隔一定次数迭代时自动记录日志。

		它也支持暂停和恢复计时语义，并且可以统计总的暂停时间。理论上，使用总的暂停时间加上总的
		流逝时间可以得到不受暂停影响的纯执行时间。
	*/

	/*!
		\since Visindigo 0.16.0
		\a autoStart 是否在创建对象时自动开始计时。
		\a name 用于日志记录的名称，默认为 "BenchmarkTimer"。
		\a autoLog 是否在计时器开始、停止、暂停和恢复时自动记录日志。

		构造函数，创建一个BenchmarkTimer对象。
	*/
	BenchmarkTimer::BenchmarkTimer(bool autoStart, const QString& name, bool autoLog) {
		d = new BenchmarkTimerPrivate();
		d->autoLog = autoLog;
		d->logger = new General::Logger(name);
		auto initTime = std::chrono::high_resolution_clock::now();
		d->startTime = initTime;
		d->pausedTime = initTime;
		if (autoStart) {
			start();
		}
	}

	/*!
		\since Visindigo 0.16.0

		析构函数，销毁BenchmarkTimer对象。

		如果计时器正在运行，则在销毁前自动执行stop()。
	*/
	BenchmarkTimer::~BenchmarkTimer() {
		if (d->isRunning) {
			stop();
		}
		delete d;
	}

	/*!
		\since Visindigo 0.16.0
		\a count 每隔多少次调用counter()方法时自动记录一次日志。

		设置自动记录日志的计数阈值。当调用counter()方法的次数达到这个阈值时，BenchmarkTimer会自动记录一次日志，显示当前的流逝时间。
		内部默认为1，即每次调用counter()都会记录日志。你可以将其设置为更大的值以减少日志记录的频率，或者设置为0以禁用自动日志记录。
	*/
	void BenchmarkTimer::setLogCount(qint32 count) {
		d->logCount = count;
	}

	/*!
		\since Visindigo 0.16.0

		return 当前的自动日志记录计数阈值。
	*/
	qint32 BenchmarkTimer::getLogCount() const {
		return d->logCount;
	}

	/*!
		\since Visindigo 0.16.0

		启动计时器。如果计时器已经在运行，则此函数不会有任何效果。
	*/
	void BenchmarkTimer::start() {
		if (not d->isRunning) {
			d->startTime = std::chrono::high_resolution_clock::now();
			d->totalPausedTime = std::chrono::duration<double>::zero();
			d->isRunning = true;
			d->isPaused = false;
			if (d->autoLog) {
				d->logger->info()<< "BenchmarkTimer started.";
			}
		}
	}

	/*!
		\since Visindigo 0.16.0

		停止计时器并返回从开始到现在的流逝时间（以毫秒为单位）。如果计时器没有在运行，则返回0.0。

		如果要访问总的暂停时间，可以调用totalPaused()方法。

		如果在停止之前，计时器处于暂停状态，则会先恢复计时器，并立即停止。这可能会产生几纳秒的误差，但通常可以忽略不计。
	*/
	double BenchmarkTimer::stop() {
		if (d->isRunning) {
			if (d->isPaused) {
				resume();
			}
			auto endTime = std::chrono::high_resolution_clock::now();
			d->isRunning = false;
			d->isPaused = false;
			d->pausedTime = endTime; // when not running ,use pausedTime as the stop time.
			auto elapsedTime = std::chrono::duration<double, std::milli>(endTime - d->startTime - d->totalPausedTime);
			if (d->autoLog) {
				d->logger->info()<< "Elapsed time: " << elapsedTime.count() << " milliseconds.";
				if (d->totalPausedTime.count() > 0) {
					d->logger->info()<< "Total paused time: " << std::chrono::duration<double, std::milli>(d->totalPausedTime).count() << " milliseconds.";
				}
			}
			return elapsedTime.count();
		}
		return 0.0;
	}

	/*!
		\since Visindigo 0.16.0

		return 从开始到现在的流逝时间（以毫秒为单位）。如果计时器正在运行，则返回一个实时的已流逝时间；
		如果计时器没有在运行，则返回上一次在运行时的最终流逝时间。

		如果定时器从未被启动过，这个函数理应返回0
	*/
	double BenchmarkTimer::elapsed() {
		if (d->isRunning) {
			auto currentTime = std::chrono::high_resolution_clock::now();
			return std::chrono::duration<double, std::milli>(currentTime - d->startTime - d->totalPausedTime).count();
		}
		else {
			return std::chrono::duration<double, std::milli>(d->pausedTime - d->startTime - d->totalPausedTime).count();
		}
	}

	/*!
		\since Visindigo 0.16.0

		调用这个函数以增加一次计数。当计数达到预设的日志记录阈值时，BenchmarkTimer会自动记录一次日志，显示当前的流逝时间。

		return 从开始到现在的流逝时间（以毫秒为单位）。如果计时器没有在运行，则返回0.0。
	*/
	double BenchmarkTimer::counter() {
		if (d->isRunning) {
			auto currentTime = std::chrono::high_resolution_clock::now();
			if (d->autoLog) {
				d->currentCount++;
				if (d->currentCount >= d->logCount) {
					d->logger->info()<< "Elapsed time: " << std::chrono::duration<double, std::milli>(currentTime - d->startTime - d->totalPausedTime).count() << " milliseconds.";
					d->currentCount = 0;
				}
			}
			return std::chrono::duration<double, std::milli>(currentTime - d->startTime - d->totalPausedTime).count();
		}
		return 0.0;
	}

	/*!
		\since Visindigo 0.16.0
		暂停计时器。如果计时器正在运行且没有处于暂停状态，则会记录当前的暂停时间，并将计时器状态设置为暂停。

		return 从开始到现在的流逝时间（以毫秒为单位）。如果计时器没有在运行，或者已经处于暂停状态，则返回0.0。
	*/
	double BenchmarkTimer::pause() {
		if (d->isRunning && not d->isPaused) {
			d->pausedTime = std::chrono::high_resolution_clock::now();
			d->isPaused = true;
			if (d->autoLog) {
				d->logger->info()<< "BenchmarkTimer paused at " << std::chrono::duration<double, std::milli>(d->pausedTime - d->startTime - d->totalPausedTime).count() << " milliseconds.";
			}
			return std::chrono::duration<double, std::milli>(d->pausedTime - d->startTime - d->totalPausedTime).count();
		}
		return 0.0;
	}

	/*!
		\since Visindigo 0.16.0
		恢复计时器。如果计时器正在运行且处于暂停状态，则会计算从上次暂停到现在的时间，并将其添加到总的暂停时间中，然后将计时器状态设置为非暂停。

		return 从开始到现在的流逝时间（以毫秒为单位）。如果计时器没有在运行，或者没有处于暂停状态，则返回0.0。
	*/
	double BenchmarkTimer::resume() {
		if (d->isRunning && d->isPaused) {
			auto resumeTime = std::chrono::high_resolution_clock::now();
			d->totalPausedTime += resumeTime - d->pausedTime;
			d->isPaused = false;
			if (d->autoLog) {
				d->logger->info()<< "BenchmarkTimer resumed at " << std::chrono::duration<double, std::milli>(resumeTime - d->startTime - d->totalPausedTime).count() << " milliseconds.";
			}
			return std::chrono::duration<double, std::milli>(resumeTime - d->startTime - d->totalPausedTime).count();
		}
		return 0.0;
	}

	/*!
		\since Visindigo 0.16.0

		重启计时器。如果计时器正在运行，则先停止计时器，然后重新开始计时。如果计时器没有在运行，则直接开始计时。

		return 从开始到现在的流逝时间（以毫秒为单位）。如果计时器没有在运行，则返回0.0。
	*/
	double BenchmarkTimer::restart() {
		if (d->isRunning) {
			auto time = stop();
			start();
			return time;
		}
		else {
			start();
			return 0.0;
		}
	}

	/*!
		\since Visindigo 0.16.0
		 
		 return 从开始到现在的总暂停时间（以毫秒为单位）。如果计时器没有在运行，
		 则返回上一次在运行时的总暂停时间。如果计时器从未被启动过，这个函数理应返回0。

		 注意，这个函数返回的总暂停时间不反应当前正在进行的暂停，因为当前的暂停时间还没有被添加到totalPausedTime中。
		 要获取当前的暂停时间，你需要在调用这个函数之前先调用resume()方法恢复计时器运行。如有需要，可以稍后再
		 重新调用pause()方法以恢复暂停状态。
	*/
	double BenchmarkTimer::totalPaused() {
		return std::chrono::duration<double, std::milli>(d->totalPausedTime).count();
	}

	/*!
		\since Visindigo 0.16.0
		return 如果计时器正在运行，则返回true；否则返回false。
	*/
	bool BenchmarkTimer::isRunning() const {
		return d->isRunning;
	}

	/*!
		\since Visindigo 0.16.0
		return 如果计时器处于暂停状态，则返回true；否则返回false。
	*/
	bool BenchmarkTimer::isPaused() const {
		return d->isPaused;
	}
}