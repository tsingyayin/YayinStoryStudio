#include "General/YSSFileAttributes.h"
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
		\inheaderfile General/YSSFileAttributes.h
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
		\value Permissions 完整的 POSIX mode。
		\value OwnerId 所有者 id。
		\value GroupId 所属组 id。
		\value LinkTarget 符号链接目标。
	*/

	/*!
		\enum YSSCore::General::YSSFileAttributes::Permission
		\since YSS 0.18.0

		权限位的具名定义，数值与 POSIX \c{st_mode} 一致。

		有关POSIX对于mode的定义，请自行另行参考，此处不再赘述。
		如果你不知道这里在讲什么，请直接搜索\c{chmod 777}，你会找到很多相关的教程。

		\value None 没有任何位。作为掩码时表示空集合。
		\value TypeMask 文件类型掩码，等价于 \c{S_IFMT}。
		\value TypeFifo 命名管道。
		\value TypeCharacterDevice 字符设备。
		\value TypeDirectory 目录。
		\value TypeBlockDevice 块设备。
		\value TypeRegular 普通文件。
		\value TypeSymlink 符号链接。
		\value TypeSocket Unix domain socket。
		\value SetUserId 执行时以文件所有者的身份运行。
		\value SetGroupId 执行时以文件所属组的身份运行；目录上表示新建文件继承该组。
		\value Sticky 目录上限制删除权限，只有条目所有者能删除自己的文件。
		\value OwnerRead 所有者可读。
		\value OwnerWrite 所有者可写。
		\value OwnerExecute 所有者可执行；目录上表示可进入。
		\value GroupRead 所属组可读。
		\value GroupWrite 所属组可写。
		\value GroupExecute 所属组可执行。
		\value OtherRead 其他人可读。
		\value OtherWrite 其他人可写。
		\value OtherExecute 其他人可执行。
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

		return 后端真正给出的字段；空对象返回 \l{Field::None}。
	*/
	YSSFileAttributes::Fields YSSFileAttributes::getKnownFields() const {
		return this->d ? this->d->KnownFields : Fields(Field::None);
	}

	/*!
		\since YSS 0.18.0

		判断单个字段是否可信。

		\a field 字段标识。传 \l{Field::None} 时返回"没有任何字段可信"。

		return 字段可信时返回 true。
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

		return 全部可信时返回 true；空集合返回 true。
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
		return 字节数；不可信时为 0。
	*/
	quint64 YSSFileAttributes::getSize() const {
		return this->d ? this->d->Size : 0;
	}

	/*!
		\since YSS 0.18.0
		return 修改时间的 UTC 毫秒数；不可信时为 0。
	*/
	qint64 YSSFileAttributes::getModifiedMs() const {
		return this->d ? this->d->ModifiedMs : 0;
	}

	/*!
		\since YSS 0.18.0
		return 创建时间的 UTC 毫秒数；不可信时为 0。
	*/
	qint64 YSSFileAttributes::getCreatedMs() const {
		return this->d ? this->d->CreatedMs : 0;
	}

	/*!
		\since YSS 0.18.0
		return 访问时间的 UTC 毫秒数；不可信时为 0。
	*/
	qint64 YSSFileAttributes::getAccessedMs() const {
		return this->d ? this->d->AccessedMs : 0;
	}

	/*!
		\since YSS 0.18.0
		return 是否为目录；不可信时为 false。
	*/
	bool YSSFileAttributes::isDirectory() const {
		return this->d ? this->d->IsDirectory : false;
	}

	/*!
		\since YSS 0.18.0
		return 是否为符号链接；不可信时为 false。
	*/
	bool YSSFileAttributes::isSymlink() const {
		return this->d ? this->d->IsSymlink : false;
	}

	/*!
		\since YSS 0.18.0
		return 是否为隐藏项；不可信时为 false。
	*/
	bool YSSFileAttributes::isHidden() const {
		return this->d ? this->d->IsHidden : false;
	}

	/*!
		\since YSS 0.18.0
		return 是否只读；不可信时为 false。
	*/
	bool YSSFileAttributes::isReadOnly() const {
		return this->d ? this->d->IsReadOnly : false;
	}

	/*!
		\since YSS 0.18.0
		return 是否可执行；不可信时为 false。
	*/
	bool YSSFileAttributes::isExecutable() const {
		return this->d ? this->d->IsExecutable : false;
	}

	/*!
		\since YSS 0.18.0

		取原生 POSIX mode，包含文件类型位与特殊位。

		return 完整 mode；不可信时为 \l{Permission::None}。
	*/
	YSSFileAttributes::Permissions YSSFileAttributes::getPermissions() const {
		return permissionsOf(this->d ? this->d->Permissions : 0);
	}

	/*!
		\since YSS 0.18.0

		取 mode 里的文件类型段（即 \c{mode & 0170000}）。

		return 某个 \c{Type*} 取值；类型未知时返回 \l{Permission::None}。
	*/
	YSSFileAttributes::Permission YSSFileAttributes::getFileType() const {
		const quint32 mode = this->d ? this->d->Permissions : 0;
		return static_cast<Permission>(mode & static_cast<quint32>(Permission::TypeMask));
	}

	/*!
		\since YSS 0.18.0
		return 所有者 id；不可信时为 0。
	*/
	quint32 YSSFileAttributes::getOwnerId() const {
		return this->d ? this->d->OwnerId : 0;
	}

	/*!
		\since YSS 0.18.0
		return 所属组 id；不可信时为 0。
	*/
	quint32 YSSFileAttributes::getGroupId() const {
		return this->d ? this->d->GroupId : 0;
	}

	/*!
		\since YSS 0.18.0
		return 符号链接目标；不可信或不是链接时为空。
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

		把 POSIX \c{st_mode} 原始值包装成 \l{Permissions}。只保留低 16 位，
		多余的高位会被丢弃，因此不要求调用方先做截断。

		\a rawMode 后端给出的 mode 原值。
	*/
	YSSFileAttributes::Permissions YSSFileAttributes::permissionsOf(quint32 rawMode) {
		return Permissions(static_cast<Permission>(rawMode & 0177777u));
	}

	/*!
		\since YSS 0.18.0

		设置原生 POSIX mode，并标记该字段可信。
	*/
	void YSSFileAttributes::setPermissions(Permissions permissions) {
		this->d->Permissions = permissions.toInt();
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

		return 至少有一个字段可信时返回 true。
	*/
	bool YSSFileAttributes::isValid() const {
		return this->d != nullptr && this->d->KnownFields != Field::None;
	}
}
