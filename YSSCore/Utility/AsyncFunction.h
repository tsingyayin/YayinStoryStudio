#pragma once
#include <QtCore/qthread.h>
#include <functional>
#include <tuple>
namespace YSSCore::Utility {
	template<typename RTN, typename ...ARGS>
	class AsyncFunction : public QThread {
		std::function<RTN(ARGS...)> Function;
		std::tuple<ARGS...> Args;
		std::function<void(RTN)> Then;
		RTN Return;
	public:
		AsyncFunction(std::function<RTN(ARGS...)> function, std::tuple<ARGS...> args, std::function<void(RTN)> then) {
			this->Function = function;
			this->Args = args;
			this->Then = then;
			QObject::connect(this, &QThread::finished, this, &AsyncFunction<RTN, ARGS...>::onFinish);
			this->start();
		};
	protected:
		AsyncFunction(const AsyncFunction& other) = delete;
		AsyncFunction(AsyncFunction&& other) = delete;
		void run() {
			[&] <size_t... N>(std::index_sequence<N...>) {
				Return = Function(std::get<N>(Args)...);
			}(std::index_sequence_for<ARGS...>());
		}
		void onFinish() {
			Then(Return);
			this->deleteLater();
		}
	};
}
#define YSSAsync new YSSCore::Utility::AsyncFunction