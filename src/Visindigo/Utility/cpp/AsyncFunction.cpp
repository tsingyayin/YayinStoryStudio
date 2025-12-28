#include "../AsyncFunction.h"
namespace Visindigo::Utility {
	/*!
		\class Visindigo::Utility::AsyncFunction
		\brief 异步函数类模板
		\since Yayin Story Studio 0.13.0
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
		\fn template <typename RTN, typename ...ARGS> Visindigo::Utility::AsyncFunction<RTN, ARGS...>::AsyncFunction(std::function<RTN(ARGS...)> function, std::tuple<ARGS...> args, std::function<void(RTN)> then)
		\a function 需要异步执行的函数
		\a args 传递给函数的参数
		\a then 函数执行完成后调用的回调函数
		\since Yayin Story Studio 0.13.0
		类的构造函数
	*/
}