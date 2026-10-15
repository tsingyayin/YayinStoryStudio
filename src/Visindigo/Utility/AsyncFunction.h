#ifndef Visindigo_Utility_AsyncFunction_h
#define Visindigo_Utility_AsyncFunction_h
#include <QThread>
#include <QPointer>
#include <QMetaObject>
#include <functional>
#include <tuple>
#include <type_traits>
namespace Visindigo::Utility {
	template<typename RTN, typename ...ARGS>
	class AsyncFunction : public QThread {
		std::function<RTN(ARGS...)> Function;
		std::tuple<ARGS...> Args;
		std::function<void(RTN)> Then;
		RTN Return;
		QPointer<QObject> Context;
		bool ContextSpecified = false;
	public:
		AsyncFunction(std::tuple<ARGS...> args, std::function<RTN(ARGS...)> function, std::function<void(RTN)> then, QObject* context = nullptr) {
			this->Function = function;
			this->Args = args;
			this->Then = then;
			this->ContextSpecified = (context != nullptr);
			this->Context = context;
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
			// 指定了上下文时，回调与上下文的生命周期绑定：上下文已经销毁就丢弃回调，
			// 否则回调会落在已经释放的控件上。RTN 可拷贝时回调才会经由上下文投递。
			if constexpr (std::is_copy_constructible_v<RTN>) {
				if (ContextSpecified) {
					if (not Context.isNull()) {
						QMetaObject::invokeMethod(Context.data(), [then = Then, ret = Return]() {
							then(ret);
						});
					}
					this->deleteLater();
					return;
				}
			}
			Then(Return);
			this->deleteLater();
		}
	};
}
#define YSSAsync new Visindigo::Utility::AsyncFunction
#endif // Visindigo_Utility_AsyncFunction_h