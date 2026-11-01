#ifndef YSSCore_General_YSSFileEntry_h
#define YSSCore_General_YSSFileEntry_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>
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
			Size = 1ull << 0,
			ModifiedMs = 1ull << 1,
			CreatedMs = 1ull << 2,
			AccessedMs = 1ull << 3,
			IsDirectory = 1ull << 4,
			IsSymlink = 1ull << 5,
			IsHidden = 1ull << 6,
			IsReadOnly = 1ull << 7,
			IsExecutable = 1ull << 8,
			Permissions = 1ull << 9,
			OwnerId = 1ull << 10,
			GroupId = 1ull << 11,
			LinkTarget = 1ull << 12
		};
		Q_ENUM(Field)
		Q_DECLARE_FLAGS(Fields, Field)
		Q_FLAG(Fields)
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
		quint32 getPermissions() const;
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
		void setPermissions(quint32 permissions);
		void setOwnerId(quint32 ownerId);
		void setGroupId(quint32 groupId);
		void setLinkTarget(const QString& linkTarget);
	public:
		bool isValid() const;
	private:
		YSSFileAttributesPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(YSSFileAttributes::Fields)

	struct YSSCoreAPI YSSFileEntry {
		QString Name;
		YSSFileAttributes Attributes;
	};
}
#endif // YSSCore_General_YSSFileEntry_h
