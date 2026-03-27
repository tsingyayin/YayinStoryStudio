#ifndef Visindigo_General_Permission_h
#define Visindigo_General_Permission_h
#include "../VICompileMacro.h"
#include <QtCore/qstring.h>
#include <QtCore/qdatetime.h>
namespace Visindigo::General {
	class PermissionPrivate;
	class Permission final {
	public:
		enum class ContextOperator {
			Equal,
			NotEqual,
			MathEqual,
			MathNotEqual,
			GreaterThan,
			LessThan,
			GreaterThanOrEqual,
			LessThanOrEqual
		};
	public:
		Permission(const QString& permNode, bool value = true, const QString& context = QString(), ContextOperator op = ContextOperator::Equal);
		VIMoveable(Permission);
		VICopyable(Permission);
		~Permission();
	public:
		QString getPermissionNode() const;
		bool getValue() const;
		QString getContext() const;
		ContextOperator getContextOperator() const;
	private:
		PermissionPrivate* d;
	};

	class PermissionManager;
	class PermissionManagerPrivate;
	class PermissionGroupPrivate;
	class PermissionGroup final {
		friend class PermissionManager;
		friend class PermissionManagerPrivate;
	protected:
		PermissionGroup(const QString& groupid, const QString& groupName);
	public:
		~PermissionGroup();
	public:
		QString getGroupId() const;
		QString getGroupName() const;
		void addPermission(const Permission& perm);
		void removePermission(const QString& permNode);
		QList<Permission> getPermissions() const;
		void setPermissions(const QList<Permission>& perms);
		void addParentGroup(const QString& parentGroupID);
		void removeParentGroup(const QString& parentGroupID);
		void setParentGroups(const QList<QString>& parentGroupIDs);
		QList<QString> getParentGroups() const;
	private:
		PermissionGroupPrivate* d;
	};

	class PermissionUserPrivate;
	class PermissionUser final {
		friend class PermissionManager;
		friend class PermissionManagerPrivate;
	protected:
		PermissionUser(const QString& userName);
	public:
		~PermissionUser();
	public:
		QString getUserName() const;
		void addPermission(const Permission& perm, const QDateTime& expiry = QDateTime());
		void removePermission(const QString& permNode);
		void addPermissionGroup(const QString& groupName, const QDateTime& expiry = QDateTime());
		void setPermissionGroups(const QList<std::tuple<QString, QDateTime>>& groups);
		void removePermissionGroup(const QString& groupName);
		QList<Permission> getPermissions() const;
		QList<QString> getPermissionGroups() const;
		bool checkPermission(const QString& permNode) const;
		void debugCheckPermission(const QString& permNode) const;
	};

	class PermissionManagerPrivate;
	class PermissionManager final {
	public:
		static PermissionManager* getInstance();
		~PermissionManager();
	private:
		PermissionManager();
	public:
		PermissionGroup* createPermissionGroup(const QString& groupName);
		PermissionGroup* getPermissionGroup(const QString& groupName) const;
		bool hasPermissionGroup(const QString& groupName) const;
		void removePermissionGroup(const QString& groupName);
		PermissionUser* createPermissionUser(const QString& roleName);
		PermissionUser* getPermissionUser(const QString& roleName) const;
		bool hasPermissionUser(const QString& roleName) const;
		void removePermissionUser(const QString& roleName);
		bool checkPermission(const QString& userName, const QString& permNode) const;
		void debugCheckPermission(const QString& userName, const QString& permNode) const;
	private:
		PermissionManagerPrivate* d;
	};
}
#endif // Visindigo_General_Permission_h
