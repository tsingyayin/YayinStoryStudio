#include "Utility/AsyncFunction.h"
namespace Visindigo::Utility {
	/*!
		\class Visindigo::Utility::AsyncFunction
		\brief 异步函数类模板.
		\since Visindigo 0.13.0
		\inmodule Visindigo

		此类提供一种便捷的函数异步方式。

		\code
		YSSAsync<int, int, int>(
			{ 3, 4 },
			[](int a, int b)->int {
				QThread::msleep(10000);
				return a + b;
			},

			[](int rtn) {
				qDebug() << rtn;
			}
		);
		\endcode
	*/

	/*!
		\fn template <typename RTN, typename ...ARGS> Visindigo::Utility::AsyncFunction<RTN, ARGS...>::AsyncFunction(std::tuple<ARGS...> args, std::function<RTN(ARGS...)> function, std::function<void(RTN)> then, QObject* context)
		\a args 传递给函数的参数
		\a function 需要异步执行的函数
		\a then 函数执行完成后调用的回调函数
		\a context 回调的上下文对象，可为nullptr
		\since Yayin Story Studio 0.13.0
		类的构造函数

		\a context 不为nullptr时，\a then 的调用与 \a context 的生命周期绑定：\a context 在此对象完成工作前被销毁，
		则 \a then 不会被调用。回调始终投递到 \a context 所属的线程，因此 \a context 必须与创建此对象的代码在同一线程中。
		\since Visindigo 0.17.0 context参数
	*/
}