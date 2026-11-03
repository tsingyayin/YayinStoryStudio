#include "General/YSSFileSystemRequest.h"
#include <QtCore/qbytearray.h>

namespace YSSCore::General {
	class YSSFileSystemRequestPrivate {
		friend class YSSFileSystemRequest;
	protected:
		YSSFileSystemRequest::Operation Operation = YSSFileSystemRequest::Operation::Unknown;
		YSSURI Uri;
		YSSURI TargetUri;
		QByteArray Data;
		bool AutoDelete = true;
	};

	/*!
		\class YSSCore::General::YSSFileSystemRequest
		\inheaderfile General/YSSFileSystemRequest.h
		\since YSS 0.18.0
		\inmodule YSSCore
		\brief UFS中的文件操作请求.
		\ingroup YSSUFS

		此类代表UFS中的文件操作请求。它包含操作类型、操作URI、目标URI（可选URI）、可选的写入内容，以及是否自动回收结果对象。

		与Reply对象不同，此类是个值类，允许复制和移动，也不包含实际的操作过程实例，只用于发起一次
		请求。因此，如果有必要，此类的实例可以反复使用。

		此类的绝大多数操作都必须设置操作URI。而在某些涉及到先后URI差异的操作中
		则必须设置目标URI（可选URI），如复制/重命名/移动操作。

		重命名指的是不改变所在文件路径的情况下变更文件名，移动更多指的是改变文件路径。
		移动请求会产生实际的数据传输，如果你使用移动却只是纯粹的做了一次重命名，效率上得不偿失。

		值得指出的是，UFS支持跨域复制/移动，但不保证能恢复。也就是说，跨域的移动操作可能会导致源文件被删除而目标文件无法恢复，
		因此绝大部分情况下，建议先复制，并自行根据文件内容对新文件进行验证，再对旧文件实施删除。

		有关UFS中，域的概念，请参见UFS类的文档。
	*/

	/*!
	\enum YSSCore::General::YSSFileSystemRequest::Operation
	\since YSS 0.18.0

	文件系统操作类型

	\value Unknown 未指定，这样的请求会被拒绝。
	\value Stat 读取属性。
	\value Read 读取整个文件。
	\value Write 整体覆盖写入。
	\value List 列出目录条目。
	\value Remove 删除文件或符号链接，或者是整个目录树（递归）。不经过回收站。
	\value Recycle 移入后端提供的回收站。需要 \c{CanRecycle} 能力，且不保证能恢复。
	\value Rename 重命名或移动，需要同时给出 YSSFileSystemRequest::setTargetUri()。
	\value Copy 复制，需要同时给出 setTargetUri()。
	\value Move 移动，需要同时给出 setTargetUri()。
	\value Mkdir 创建目录。
	\value Rmdir 删除空目录。目录非空时失败，需要连内容一起删请用 Remove。
*/

	/*!
		\since YSS 0.18.0

		构造一个空请求。
	*/
	YSSFileSystemRequest::YSSFileSystemRequest() {
		this->d = new YSSFileSystemRequestPrivate();
	}

	VICopyable_Impl(YSSFileSystemRequest);
	VIMoveable_Impl(YSSFileSystemRequest);

	/*!
		\since YSS 0.18.0

		析构函数。
	*/
	YSSFileSystemRequest::~YSSFileSystemRequest() {
		delete this->d;
	}

	/*!
		\since YSS 0.18.0

		设置操作类型。

		\a operation 操作类型。

		return 自身引用。
	*/
	YSSFileSystemRequest& YSSFileSystemRequest::setOperation(YSSFileSystemRequest::Operation operation) {
		this->d->Operation = operation;
		return *this;
	}

	/*!
		\since YSS 0.18.0

		设置操作目标的绝对 URI。

		\a uri 目标 URI。

		return 自身引用。
	*/
	YSSFileSystemRequest& YSSFileSystemRequest::setUri(const YSSURI& uri) {
		this->d->Uri = uri;
		return *this;
	}

	/*!
		\since YSS 0.18.0

		设置重命名/移动的绝对目标 URI。

		\a uri 目标 URI。

		return 自身引用。
	*/
	YSSFileSystemRequest& YSSFileSystemRequest::setTargetUri(const YSSURI& uri) {
		this->d->TargetUri = uri;
		return *this;
	}

	/*!
		\since YSS 0.18.0

		设置待写入内容，仅 YSSFileSystemRequest::Operation::Write 使用。

		\a data 内容。

		return 自身引用。
	*/
	YSSFileSystemRequest& YSSFileSystemRequest::setData(const QByteArray& data) {
		this->d->Data = data;
		return *this;
	}

	/*!
		\since YSS 0.18.0

		设置结果对象是否自动回收。

		\a autoDelete 是否自动回收。

		return 自身引用。
	*/
	YSSFileSystemRequest& YSSFileSystemRequest::setAutoDelete(bool autoDelete) {
		this->d->AutoDelete = autoDelete;
		return *this;
	}

	/*!
		\since YSS 0.18.0

		取操作类型。

		return 操作类型。
	*/
	YSSFileSystemRequest::Operation YSSFileSystemRequest::getOperation() const {
		return this->d->Operation;
	}

	/*!
		\since YSS 0.18.0

		取操作目标的绝对 URI。

		return 目标 URI；未设置时为空。
	*/
	YSSURI YSSFileSystemRequest::getUri() const {
		return this->d->Uri;
	}

	/*!
		\since YSS 0.18.0

		取重命名/移动的绝对目标 URI。

		return 目标 URI；未设置时为空。
	*/
	YSSURI YSSFileSystemRequest::getTargetUri() const {
		return this->d->TargetUri;
	}

	/*!
		\since YSS 0.18.0

		判断本请求是否用绝对 URI 指定目标。

		return 设置了 URI 时返回 true。
	*/
	bool YSSFileSystemRequest::hasUri() const {
		return this->d->Uri.isValid();
	}

	/*!
		\since YSS 0.18.0

		判断本请求是否用绝对 URI 指定重命名目标。

		return 设置了目标 URI 时返回 true。
	*/
	bool YSSFileSystemRequest::hasTargetUri() const {
		return this->d->TargetUri.isValid();
	}

	/*!
		\since YSS 0.18.0

		取待写入内容。

		return 内容。
	*/
	QByteArray YSSFileSystemRequest::getData() const {
		return this->d->Data;
	}

	/*!
		\since YSS 0.18.0

		判断结果对象是否自动回收。

		return 自动回收时返回 true。
	*/
	bool YSSFileSystemRequest::isAutoDelete() const {
		return this->d->AutoDelete;
	}

	/*!
		\since YSS 0.18.0

		判断参数是否自洽，即必须有操作类型，重命名、复制、移动必须有目标 URI（即可选URI）。

		return 参数自洽时返回 true。
	*/
	bool YSSFileSystemRequest::isValid() const {
		if (this->d->Operation == YSSFileSystemRequest::Operation::Unknown) {
			return false;
		}
		switch (this->d->Operation) {
		case YSSFileSystemRequest::Operation::Rename:
		case YSSFileSystemRequest::Operation::Copy:
		case YSSFileSystemRequest::Operation::Move:
			if (!this->d->TargetUri.isValid()) {
				return false;
			}
			break;
		default:
			break;
		}
		return true;
	}
}
