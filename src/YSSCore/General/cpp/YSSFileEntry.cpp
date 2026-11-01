#include "General/YSSFileEntry.h"
#include <QtCore/qstring.h>

namespace YSSCore::General {
	class YSSFileAttributesPrivate {
		friend class YSSFileAttributes;
	protected:
		YSSFileAttributes::Fields KnownFields = YSSFileAttributes::Field::None;
		quint64 Size = 0;
		qint64 ModifiedMs = 0;
		qint64 CreatedMs = 0;
		qint64 AccessedMs = 0;
		bool IsDirectory = false;
		bool IsSymlink = false;
		bool IsHidden = false;
		bool IsReadOnly = false;
		bool IsExecutable = false;
		quint32 Permissions = 0;
		quint32 OwnerId = 0;
		quint32 GroupId = 0;
		QString LinkTarget;
	};

	/*!
		\class YSSCore::General::YSSFileAttributes
		\inheaderfile General/YSSFileEntry.h
		\since YSS 0.18.0
		\inmodule YSSCore
		\brief UFS中对文件属性的表示
		\ingroup YSSUFS

		此类是UFS中，用于提供文件相关属性的类。

		由于UFS的后端是多样的，不同后端可能提供不同的属性支持，因此
		在使用本类时，建议使用 \l{Field} 位掩码来判断某个字段是否可信。

		一般来说，如果UFS访问一个本机URI，则此类的绝大部分字段应该都可用，
		且应该与原生API提供一致的语义和值。但如果是访问远端URI，则一般
		只有少数几个字段可用，且值可能与原生API不一致。譬如，远端的ModifiedMs
		可能只能精确到秒而非毫秒。

		与此同时，考虑到UFS面对的本地+SSH混合场景的后者其实更常在
		使用Linux或类Unix系统，因此显而易见的此类的语义更偏向于POSIX，
		Permission、OwnerID、GroupId属性就是很好的例子。

		\list
		\li \l{Field::Size} 字节数；目录为 0。
		\li \l{Field::ModifiedMs} 修改时间的 UTC 毫秒数。
		\li \l{Field::CreatedMs} 创建时间的 UTC 毫秒数；SFTP 给不出，通常只有本机可用。
		\li \l{Field::AccessedMs} 最后访问时间的 UTC 毫秒数。
		\li \l{Field::IsDirectory} 是否为目录。
		\li \l{Field::IsSymlink} 是否为符号链接。
		\li \l{Field::IsHidden} 是否为隐藏项。本机取平台隐藏属性，远端按 POSIX
			的点号前缀约定。
		\li \l{Field::IsReadOnly} 是否只读。本机取当前进程的有效权限，远端只能按
			权限位推断（没有写位即为只读）。
		\li \l{Field::IsExecutable} 是否可执行。本机取平台判定，远端按权限位推断。
		\li \l{Field::Permissions} 原生权限位。本机由 \l{QFileDevice::Permissions}
			映射成 POSIX mode，远端为 SFTP 原样给出的 mode。
		\li \l{Field::OwnerId} / \l{Field::GroupId} 所属用户 / 组的数字 id。
		\li \l{Field::LinkTarget} 符号链接指向的路径。
		\endlist
	*/

	/*!
		\enum YSSCore::General::YSSFileAttributes::Field
		\since YSS 0.18.0

		属性的字段标识，同时用作"哪些字段可信"的位掩码。

		\value None 没有字段。作为掩码时表示空集合。
		\value Size 大小。
		\value ModifiedMs 修改时间。
		\value CreatedMs 创建时间。
		\value AccessedMs 访问时间。
		\value IsDirectory 目录标志。
		\value IsSymlink 符号链接标志。
		\value IsHidden 隐藏标志。
		\value IsReadOnly 只读标志。
		\value IsExecutable 可执行标志。
		\value Permissions 原生权限位。
		\value OwnerId 所有者 id。
		\value GroupId 所属组 id。
		\value LinkTarget 符号链接目标。
	*/

	/*!
		\since YSS 0.18.0

		构造一个空属性对象：所有字段都不可信。
	*/
	YSSFileAttributes::YSSFileAttributes() {
		this->d = new YSSFileAttributesPrivate();
	}

	VICopyable_Impl(YSSFileAttributes);
	VIMoveable_Impl(YSSFileAttributes);

	/*!
		\since YSS 0.18.0

		析构函数。
	*/
	YSSFileAttributes::~YSSFileAttributes() {
		delete this->d;
	}

	/*!
		\since YSS 0.18.0

		取可信字段集合。

		\return 后端真正给出的字段；空对象返回 \l{Field::None}。
	*/
	YSSFileAttributes::Fields YSSFileAttributes::getKnownFields() const {
		return this->d ? this->d->KnownFields : Fields(Field::None);
	}

	/*!
		\since YSS 0.18.0

		判断单个字段是否可信。

		\a field 字段标识。传 \l{Field::None} 时返回"没有任何字段可信"。

		\return 字段可信时返回 true。
	*/
	bool YSSFileAttributes::hasField(Field field) const {
		if (this->d == nullptr) {
			return false;
		}
		if (field == Field::None) {
			return this->d->KnownFields == Field::None;
		}
		return this->d->KnownFields.testFlag(field);
	}

	/*!
		\since YSS 0.18.0

		判断一组字段是否全部可信。

		\a fields 字段集合。

		\return 全部可信时返回 true；空集合返回 true。
	*/
	bool YSSFileAttributes::hasFields(Fields fields) const {
		if (this->d == nullptr) {
			return false;
		}
		return (this->d->KnownFields & fields) == fields;
	}

	/*!
		\since YSS 0.18.0

		手工调整某个字段的可信状态。后端的适配层在字段缺失时用它把 setter
		写入的值重新标记为不可信。

		\a field 字段标识，不能是 \l{Field::None}。
		\a known 是否可信。
	*/
	void YSSFileAttributes::setFieldKnown(Field field, bool known) {
		if (this->d == nullptr || field == Field::None) {
			return;
		}
		this->d->KnownFields.setFlag(field, known);
	}

	/*!
		\since YSS 0.18.0
		\return 字节数；不可信时为 0。
	*/
	quint64 YSSFileAttributes::getSize() const {
		return this->d ? this->d->Size : 0;
	}

	/*!
		\since YSS 0.18.0
		\return 修改时间的 UTC 毫秒数；不可信时为 0。
	*/
	qint64 YSSFileAttributes::getModifiedMs() const {
		return this->d ? this->d->ModifiedMs : 0;
	}

	/*!
		\since YSS 0.18.0
		\return 创建时间的 UTC 毫秒数；不可信时为 0。
	*/
	qint64 YSSFileAttributes::getCreatedMs() const {
		return this->d ? this->d->CreatedMs : 0;
	}

	/*!
		\since YSS 0.18.0
		\return 访问时间的 UTC 毫秒数；不可信时为 0。
	*/
	qint64 YSSFileAttributes::getAccessedMs() const {
		return this->d ? this->d->AccessedMs : 0;
	}

	/*!
		\since YSS 0.18.0
		\return 是否为目录；不可信时为 false。
	*/
	bool YSSFileAttributes::isDirectory() const {
		return this->d ? this->d->IsDirectory : false;
	}

	/*!
		\since YSS 0.18.0
		\return 是否为符号链接；不可信时为 false。
	*/
	bool YSSFileAttributes::isSymlink() const {
		return this->d ? this->d->IsSymlink : false;
	}

	/*!
		\since YSS 0.18.0
		\return 是否为隐藏项；不可信时为 false。
	*/
	bool YSSFileAttributes::isHidden() const {
		return this->d ? this->d->IsHidden : false;
	}

	/*!
		\since YSS 0.18.0
		\return 是否只读；不可信时为 false。
	*/
	bool YSSFileAttributes::isReadOnly() const {
		return this->d ? this->d->IsReadOnly : false;
	}

	/*!
		\since YSS 0.18.0
		\return 是否可执行；不可信时为 false。
	*/
	bool YSSFileAttributes::isExecutable() const {
		return this->d ? this->d->IsExecutable : false;
	}

	/*!
		\since YSS 0.18.0
		\return 原生权限位；不可信时为 0。
	*/
	quint32 YSSFileAttributes::getPermissions() const {
		return this->d ? this->d->Permissions : 0;
	}

	/*!
		\since YSS 0.18.0
		\return 所有者 id；不可信时为 0。
	*/
	quint32 YSSFileAttributes::getOwnerId() const {
		return this->d ? this->d->OwnerId : 0;
	}

	/*!
		\since YSS 0.18.0
		\return 所属组 id；不可信时为 0。
	*/
	quint32 YSSFileAttributes::getGroupId() const {
		return this->d ? this->d->GroupId : 0;
	}

	/*!
		\since YSS 0.18.0
		\return 符号链接目标；不可信或不是链接时为空。
	*/
	QString YSSFileAttributes::getLinkTarget() const {
		return this->d ? this->d->LinkTarget : QString();
	}

	/*!
		\since YSS 0.18.0

		设置大小，并标记该字段可信。
	*/
	void YSSFileAttributes::setSize(quint64 size) {
		this->d->Size = size;
		this->d->KnownFields |= Field::Size;
	}

	/*!
		\since YSS 0.18.0

		设置修改时间，并标记该字段可信。
	*/
	void YSSFileAttributes::setModifiedMs(qint64 modifiedMs) {
		this->d->ModifiedMs = modifiedMs;
		this->d->KnownFields |= Field::ModifiedMs;
	}

	/*!
		\since YSS 0.18.0

		设置创建时间，并标记该字段可信。
	*/
	void YSSFileAttributes::setCreatedMs(qint64 createdMs) {
		this->d->CreatedMs = createdMs;
		this->d->KnownFields |= Field::CreatedMs;
	}

	/*!
		\since YSS 0.18.0

		设置访问时间，并标记该字段可信。
	*/
	void YSSFileAttributes::setAccessedMs(qint64 accessedMs) {
		this->d->AccessedMs = accessedMs;
		this->d->KnownFields |= Field::AccessedMs;
	}

	/*!
		\since YSS 0.18.0

		设置目录标志，并标记该字段可信。
	*/
	void YSSFileAttributes::setIsDirectory(bool isDirectory) {
		this->d->IsDirectory = isDirectory;
		this->d->KnownFields |= Field::IsDirectory;
	}

	/*!
		\since YSS 0.18.0

		设置符号链接标志，并标记该字段可信。
	*/
	void YSSFileAttributes::setIsSymlink(bool isSymlink) {
		this->d->IsSymlink = isSymlink;
		this->d->KnownFields |= Field::IsSymlink;
	}

	/*!
		\since YSS 0.18.0

		设置隐藏标志，并标记该字段可信。
	*/
	void YSSFileAttributes::setIsHidden(bool isHidden) {
		this->d->IsHidden = isHidden;
		this->d->KnownFields |= Field::IsHidden;
	}

	/*!
		\since YSS 0.18.0

		设置只读标志，并标记该字段可信。
	*/
	void YSSFileAttributes::setIsReadOnly(bool isReadOnly) {
		this->d->IsReadOnly = isReadOnly;
		this->d->KnownFields |= Field::IsReadOnly;
	}

	/*!
		\since YSS 0.18.0

		设置可执行标志，并标记该字段可信。
	*/
	void YSSFileAttributes::setIsExecutable(bool isExecutable) {
		this->d->IsExecutable = isExecutable;
		this->d->KnownFields |= Field::IsExecutable;
	}

	/*!
		\since YSS 0.18.0

		设置原生权限位，并标记该字段可信。
	*/
	void YSSFileAttributes::setPermissions(quint32 permissions) {
		this->d->Permissions = permissions;
		this->d->KnownFields |= Field::Permissions;
	}

	/*!
		\since YSS 0.18.0

		设置所有者 id，并标记该字段可信。
	*/
	void YSSFileAttributes::setOwnerId(quint32 ownerId) {
		this->d->OwnerId = ownerId;
		this->d->KnownFields |= Field::OwnerId;
	}

	/*!
		\since YSS 0.18.0

		设置所属组 id，并标记该字段可信。
	*/
	void YSSFileAttributes::setGroupId(quint32 groupId) {
		this->d->GroupId = groupId;
		this->d->KnownFields |= Field::GroupId;
	}

	/*!
		\since YSS 0.18.0

		设置符号链接目标，并标记该字段可信。
	*/
	void YSSFileAttributes::setLinkTarget(const QString& linkTarget) {
		this->d->LinkTarget = linkTarget;
		this->d->KnownFields |= Field::LinkTarget;
	}

	/*!
		\since YSS 0.18.0

		判断属性是否携带了有效信息。

		\return 至少有一个字段可信时返回 true。
	*/
	bool YSSFileAttributes::isValid() const {
		return this->d != nullptr && this->d->KnownFields != Field::None;
	}

	/*!
		\struct YSSCore::General::YSSFileEntry
		\inheaderfile General/YSSFileEntry.h
		\since YSS 0.18.0
		\inmodule YSSCore
		\brief 目录列表中的一项.

		\list
		\li \b{Name} 名称，不含路径。
		\li \b{Attributes} 该项的属性。
		\endlist
	*/
}
