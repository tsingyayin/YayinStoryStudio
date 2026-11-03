#include "General/YSSFileSystemTask.h"
#include "General/private/YSSFileSystemTask_p.h"
#include <QtCore/qglobal.h>

namespace YSSCore::General {
	/*!
		\class YSSCore::General::YSSFileSystemTask
		\inheaderfile General/YSSFileSystemTask.h
		\since YSS 0.18.0
		\inmodule YSSCore
		\ingroup YSSUFS
		\brief UFS 中一个尚未结束的任务.

		此类代表UFS中一个已经提交，且尚未结束的任务。它是一个只读值类型对象，不与
		所代表的任务生命周期绑定，因此可以任意的复制，存储，取用它。

		这类是只读的，数据由UFS内部填充。保留了公开的默认构造函数，但默认构造没有任何实际意义。
	*/

	/*!
		\since YSS 0.18.0

		默认构造函数，构造一个无意义的空对象。
	*/
	YSSFileSystemTask::YSSFileSystemTask() {
		this->d = new YSSFileSystemTaskPrivate();
	}

	/*!
		\fn YSSFileSystemTask::YSSFileSystemTask(const YSSFileSystemTask& other)
		\since YSS 0.18.0

		拷贝构造函数。
	*/

	/*!
		\fn YSSFileSystemTask::YSSFileSystemTask(YSSFileSystemTask&& other) noexcept
		\since YSS 0.18.0

		移动构造函数。
	*/

	/*!
		\fn YSSFileSystemTask& YSSFileSystemTask::operator=(const YSSFileSystemTask& other)
		\since YSS 0.18.0

		拷贝赋值运算符。

		\a other 要拷贝的源对象。
	*/

	/*!
		\fn YSSFileSystemTask& YSSFileSystemTask::operator=(YSSFileSystemTask&& other) noexcept
		\since YSS 0.18.0

		移动赋值运算符。

		\a other 要移动的源对象。
	*/

	VICopyable_Impl(YSSFileSystemTask);
	VIMoveable_Impl(YSSFileSystemTask);

	/*!
		\since YSS 0.18.0

		析构函数。
	*/
	YSSFileSystemTask::~YSSFileSystemTask() {
		delete this->d;
	}

	/*!
		\since YSS 0.18.0

		取任务编号。

		return 与 YSSFileSystemReply::getId() 相同；空任务为 0。
	*/
	quint64 YSSFileSystemTask::getId() const {
		return this->d ? this->d->Id : 0;
	}

	/*!
		\since YSS 0.18.0

		取操作目标。

		return 提交时指定的绝对 URI；空任务为空 URI。
	*/
	YSSURI YSSFileSystemTask::getUri() const {
		return this->d ? this->d->Uri : YSSURI();
	}

	/*!
		\since YSS 0.18.0

		取操作目标的目标 URI。

		return 重命名、复制、移动的目标；其它操作或空任务为空 URI。
	*/
	YSSURI YSSFileSystemTask::getTargetUri() const {
		return this->d ? this->d->TargetUri : YSSURI();
	}

	/*!
		\since YSS 0.18.0

		取操作类型。

		return 操作类型；空任务为 YSSFileSystemRequest::Operation::Unknown。
	*/
	YSSFileSystemRequest::Operation YSSFileSystemTask::getOperation() const {
		return this->d ? this->d->Operation : YSSFileSystemRequest::Operation::Unknown;
	}

	/*!
		\since YSS 0.18.0

		取任务被受理的时间。

		return UTC 毫秒时间戳；空任务为 0。
	*/
	qint64 YSSFileSystemTask::getQueuedAtMs() const {
		return this->d ? this->d->QueuedAtMs : 0;
	}

	/*!
		\since YSS 0.18.0

		取任务开始执行的时间。

		return UTC 毫秒时间戳；0 表示仍在排队，见 isRunning()。
	*/
	qint64 YSSFileSystemTask::getStartedAtMs() const {
		return this->d ? this->d->StartedAtMs : 0;
	}

	/*!
		\since YSS 0.18.0

		取已传输的字节数。

		return 当前进度对应的字节数；没有进度信息时为 0。
	*/
	qint64 YSSFileSystemTask::getBytesTransferred() const {
		return this->d ? this->d->BytesTransferred : 0;
	}

	/*!
		\since YSS 0.18.0

		取本次传输的总字节数。

		return 总长度；-1 表示后端没有给出。
	*/
	qint64 YSSFileSystemTask::getTotalBytes() const {
		return this->d ? this->d->TotalBytes : -1;
	}

	/*!
		\since YSS 0.18.0

		判断任务是否已经开始执行。

		return 已开始返回 true。
	*/
	bool YSSFileSystemTask::isRunning() const {
		return this->d && this->d->StartedAtMs != 0;
	}

	/*!
		\since YSS 0.18.0

		计算进度百分比。

		return 0.0~100.0；总长度未知或为 0 时返回 -1.0。
	*/
	qreal YSSFileSystemTask::progressPercent() const {
		if (!this->d || this->d->TotalBytes <= 0) {
			return -1.0;
		}
		const qint64 clamped = qBound<qint64>(0, this->d->BytesTransferred, this->d->TotalBytes);
		return static_cast<qreal>(clamped) * 100.0 / static_cast<qreal>(this->d->TotalBytes);
	}
}
