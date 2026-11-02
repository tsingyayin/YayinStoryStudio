#ifndef YSSCore_General_YSSFileAttributes_h
#define YSSCore_General_YSSFileAttributes_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>
#include <VIMacro.h>
// Forward declarations
namespace YSSCore::General {
	class YSSFileAttributesPrivate;
}
// Main
namespace YSSCore::General {
	class YSSCoreAPI YSSFileAttributes {
		Q_GADGET;
	public:
		enum class Field : quint64 {
			None = 0,
			Size = VFlagU64(0),
			ModifiedMs = VFlagU64(1),
			CreatedMs = VFlagU64(2),
			AccessedMs = VFlagU64(3),
			IsDirectory = VFlagU64(4),
			IsSymlink = VFlagU64(5),
			IsHidden = VFlagU64(6),
			IsReadOnly = VFlagU64(7),
			IsExecutable = VFlagU64(8),
			Permissions = VFlagU64(9),
			OwnerId = VFlagU64(10),
			GroupId = VFlagU64(11),
			LinkTarget = VFlagU64(12)
		};
		Q_ENUM(Field)
		Q_DECLARE_FLAGS(Fields, Field)
		Q_FLAG(Fields)
	public:
		enum class Permission : quint32 {
			None = 0,
			TypeMask = 0170000,
			TypeFifo = 0010000,
			TypeCharacterDevice = 0020000,
			TypeDirectory = 0040000,
			TypeBlockDevice = 0060000,
			TypeRegular = 0100000,
			TypeSymlink = 0120000,
			TypeSocket = 0140000,
			SetUserId = 0004000,
			SetGroupId = 0002000,
			Sticky = 0001000,
			OwnerRead = 0000400,
			OwnerWrite = 0000200,
			OwnerExecute = 0000100,
			OwnerAll = OwnerRead | OwnerWrite | OwnerExecute,
			GroupRead = 0000040,
			GroupWrite = 0000020,
			GroupExecute = 0000010,
			GroupAll = GroupRead | GroupWrite | GroupExecute,
			OtherRead = 0000004,
			OtherWrite = 0000002,
			OtherExecute = 0000001,
			OtherAll = OtherRead | OtherWrite | OtherExecute,
		};
		Q_ENUM(Permission)
		Q_DECLARE_FLAGS(Permissions, Permission)
		Q_FLAG(Permissions)
	public:
		YSSFileAttributes();
		VICopyable(YSSFileAttributes);
		VIMoveable(YSSFileAttributes);
		~YSSFileAttributes();
	public:
		Fields getKnownFields() const;
		bool hasField(Field field) const;
		bool hasFields(Fields fields) const;
		void setFieldKnown(Field field, bool known);
	public:
		quint64 getSize() const;
		qint64 getModifiedMs() const;
		qint64 getCreatedMs() const;
		qint64 getAccessedMs() const;
		bool isDirectory() const;
		bool isSymlink() const;
		bool isHidden() const;
		bool isReadOnly() const;
		bool isExecutable() const;
		Permissions getPermissions() const;
		Permission getFileType() const;
		quint32 getOwnerId() const;
		quint32 getGroupId() const;
		QString getLinkTarget() const;
	public:
		void setSize(quint64 size);
		void setModifiedMs(qint64 modifiedMs);
		void setCreatedMs(qint64 createdMs);
		void setAccessedMs(qint64 accessedMs);
		void setIsDirectory(bool isDirectory);
		void setIsSymlink(bool isSymlink);
		void setIsHidden(bool isHidden);
		void setIsReadOnly(bool isReadOnly);
		void setIsExecutable(bool isExecutable);
		static Permissions permissionsOf(quint32 rawMode);
	public:
		void setPermissions(Permissions permissions);
		void setOwnerId(quint32 ownerId);
		void setGroupId(quint32 groupId);
		void setLinkTarget(const QString& linkTarget);
	public:
		bool isValid() const;
	private:
		YSSFileAttributesPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(YSSFileAttributes::Fields)
	Q_DECLARE_OPERATORS_FOR_FLAGS(YSSFileAttributes::Permissions)
}
#endif // YSSCore_General_YSSFileAttributes_h
