#include "General/YSSFileError.h"
#include <QtCore/qmetaobject.h>
#include <QtCore/qstring.h>

namespace YSSCore::General {
	class YSSFileErrorPrivate {
		friend class YSSFileError;
	protected:
		YSSFileError::ErrorType Type = YSSFileError::ErrorType::NoError;
		QString Message;
		QString Detail;
	};

	/*!
		\class YSSCore::General::YSSFileError
		\inheaderfile General/YSSFileError.h
		\since YSS 0.18.0
		\inmodule YSSCore
		\ingroup YSSUFS
		\brief UFS中对文件操作错误的表示.

		此类在UFS中，表达一种操作上的异常。它包含错误类型、错误消息和可选的详细信息。
		值类型 + PIMPL，可以自由拷贝，也会按值存进 YSSFileSystemResult。
	*/

	/*!
		\enum YSSCore::General::YSSFileError::ErrorType
		\since YSS 0.18.0

		错误分类。

		\value NoError 无错误。
		\value NotFound 文件或目录不存在。
		\value PermissionDenied 权限不足。
		\value AlreadyExists 目标已存在（例如创建已存在的目录）。
		\value NotADirectory 路径中有一段不是目录。
		\value IoError 底层读写失败。
		\value NotSupported 当前后端不支持该操作，参见 YSSUnifiedFileSystem::getCapabilities()。
		\value Aborted 操作在完成前被中止（会话关闭、对象销毁等）。
		\value InvalidConfiguration 路径或参数不合法。
		\value FileTooLarge 文件超过后端配置的大小上限。
		\value NeedsCredentials 连接当前不可用，需要用户提供凭据。
		\value Unknown 未归类的错误。
	*/

	/*!
		\since YSS 0.18.0

		构造一个"没有错误"的对象：类型为 ErrorType::NoError，消息与详情为空。
	*/
	YSSFileError::YSSFileError() {
		this->d = new YSSFileErrorPrivate();
	}

	VICopyable_Impl(YSSFileError);
	VIMoveable_Impl(YSSFileError);

	/*!
		\since YSS 0.18.0

		析构函数。
	*/
	YSSFileError::~YSSFileError() {
		delete this->d;
	}

	/*!
		\since YSS 0.18.0

		返回枚举值对应的名字字符串；传入未收录的取值时返回 \c "Unknown"。

		\a type 错误类型。
	*/
	QString YSSFileError::errorTypeName(ErrorType type) {
		const QMetaEnum metaEnum = QMetaEnum::fromType<ErrorType>();
		const char* key = metaEnum.valueToKey(static_cast<int>(type));
		return key != nullptr ? QString::fromUtf8(key) : QStringLiteral("Unknown");
	}

	/*!
		\since YSS 0.18.0

		取错误分类。

		return 错误类型；没有错误时为 ErrorType::NoError。
	*/
	YSSFileError::ErrorType YSSFileError::getType() const {
		return this->d ? this->d->Type : ErrorType::NoError;
	}

	/*!
		\since YSS 0.18.0

		取面向用户的消息。

		return 简短描述；未设置时为空串。
	*/
	QString YSSFileError::getMessage() const {
		return this->d ? this->d->Message : QString();
	}

	/*!
		\since YSS 0.18.0

		取后端给出的原始信息，用于排查。

		return 详情；未设置时为空串。
	*/
	QString YSSFileError::getDetail() const {
		return this->d ? this->d->Detail : QString();
	}

	/*!
		\since YSS 0.18.0

		设置错误分类。

		\a type 错误类型；传 ErrorType::NoError 表示清除错误。
	*/
	void YSSFileError::setType(ErrorType type) {
		this->d->Type = type;
	}

	/*!
		\since YSS 0.18.0

		设置面向用户的消息。

		\a message 简短描述。
	*/
	void YSSFileError::setMessage(const QString& message) {
		this->d->Message = message;
	}

	/*!
		\since YSS 0.18.0

		设置后端给出的原始信息。

		\a detail 详情，可以留空。
	*/
	void YSSFileError::setDetail(const QString& detail) {
		this->d->Detail = detail;
	}

	/*!
		\since YSS 0.18.0

		判断本对象是否表示一个错误。

		return 类型不是 ErrorType::NoError 时返回 true。
	*/
	bool YSSFileError::isError() const {
		return this->d && this->d->Type != ErrorType::NoError;
	}

	/*!
		\since YSS 0.18.0

		生成适合写入日志的单行描述。

		return 形如 \c{[NotFound] no such file: a.txt} 的文本。
	*/
	QString YSSFileError::toString() const {
		return QStringLiteral("[%1] %2").arg(errorTypeName(getType()), getMessage());
	}
}
