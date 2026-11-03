#include "General/YSSFileEntry.h"
#include <QtCore/qstring.h>

namespace YSSCore::General {
	// 只在本文档单元里使用，按规范直接内联在 cpp 中，不单独开私有头。
	class YSSFileEntryPrivate {
		friend class YSSFileEntry;
	protected:
		QString Name;
		YSSURI Uri;
		YSSFileAttributes Attributes;
	};

	/*!
		\class YSSCore::General::YSSFileEntry
		\inheaderfile General/YSSFileEntry.h
		\since YSS 0.18.0
		\inmodule YSSCore
		\ingroup YSSUFS
		\brief 一份完整的文件信息.

		此类包括文件的文件名、URI和文件属性，用于代表除了文件内容之外的完整文件信息。
	*/

	/*!
		\since YSS 0.18.0

		构造一个空条目：名字为空、属性全部不可信。
	*/
	YSSFileEntry::YSSFileEntry() {
		this->d = new YSSFileEntryPrivate();
	}

	/*!
		\fn YSSFileEntry::YSSFileEntry(const YSSFileEntry& other)
		\since YSS 0.18.0

		拷贝构造函数。
	*/

	/*!
		\fn YSSFileEntry::YSSFileEntry(YSSFileEntry&& other)
		\since YSS 0.18.0

		移动构造函数。
	*/
	
	/*!
		\fn YSSFileEntry& YSSFileEntry::operator=(const YSSFileEntry& other)
		\since YSS 0.18.0

		拷贝赋值运算符。

		\a other 另一个条目。
	*/

	/*!
		\fn YSSFileEntry& YSSFileEntry::operator=(YSSFileEntry&& other)
		\since YSS 0.18.0

		移动赋值运算符。

		\a other 另一个条目。
	*/

	VICopyable_Impl(YSSFileEntry);
	VIMoveable_Impl(YSSFileEntry);

	/*!
		\since YSS 0.18.0

		析构函数。
	*/
	YSSFileEntry::~YSSFileEntry() {
		delete this->d;
	}

	/*!
		\since YSS 0.18.0

		取条目名。

		return 叶子名；空条目的名字为空串。
	*/
	QString YSSFileEntry::getName() const {
		return this->d ? this->d->Name : QString();
	}

	/*!
		\since YSS 0.18.0

		取条目 URI。

		return 条目的绝对 URI；未设置时返回空 URI。
	*/
	YSSURI YSSFileEntry::getUri() const {
		return this->d ? this->d->Uri : YSSURI();
	}

	/*!
		\since YSS 0.18.0

		取条目属性。

		return 属性副本；空条目返回全部字段不可信的空属性。
	*/
	YSSFileAttributes YSSFileEntry::getAttributes() const {
		return this->d ? this->d->Attributes : YSSFileAttributes();
	}

	/*!
		\since YSS 0.18.0

		设置条目名。

		\a name 叶子名，不含路径。
	*/
	void YSSFileEntry::setName(const QString& name) {
		this->d->Name = name;
	}

	/*!
		\since YSS 0.18.0

		设置条目 URI。

		\a uri 条目的绝对 URI，通常由父目录的 URI 与叶子名 YSSURI::resolve() 得到。
	*/
	void YSSFileEntry::setUri(const YSSURI& uri) {
		this->d->Uri = uri;
	}

	/*!
		\since YSS 0.18.0

		设置条目属性。

		\a attributes 该项的属性。
	*/
	void YSSFileEntry::setAttributes(const YSSFileAttributes& attributes) {
		this->d->Attributes = attributes;
	}

	/*!
		\since YSS 0.18.0

		判断条目是否可用。

		return 名字非空时返回 true。属性为空、URI 未设置都不影响判断：一个空文件同样没有属性。
	*/
	bool YSSFileEntry::isValid() const {
		return this->d != nullptr && !this->d->Name.isEmpty();
	}
}
