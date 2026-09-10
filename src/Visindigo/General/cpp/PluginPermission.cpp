#include <QtCore/qmap.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qset.h>
#include <QtCore/qstringlist.h>
#include "General/Log.h"
#include "General/PluginPermission.h"
#include "General/VIApplication.h"
#include "Utility/Console.h"
#include "Utility/JsonConfig.h"

namespace Visindigo::General {
	// 权限记录寄存在主程序插件的配置中，结构为：
	// _visindigo_auto_.Permissions.<插件ID>.Allowed / .Rejected
	// 两个数组都以权限名（Plugin::Permission的QMetaEnum键名）保存，便于用户阅读与撤改。
	static const QString PermissionConfigRoot = QStringLiteral("_visindigo_auto_.Permissions");

	// 由权限集合得到权限名列表。Unknown（0）不记录。
	static QStringList permissionNamesOf(Plugin::Permissions perms) {
		const QMetaEnum metaEnum = QMetaEnum::fromType<Plugin::Permission>();
		QStringList names;
		for (int i = 0; i < metaEnum.keyCount(); ++i) {
			const int value = metaEnum.value(i);
			if (value == 0) {
				continue;
			}
			if (perms.testFlag(static_cast<Plugin::Permission>(value))) {
				names.append(QString::fromLatin1(metaEnum.key(i)));
			}
		}
		return names;
	}

	// 由权限名列表还原权限集合。无法识别的名字（例如权限改名或降级后的残留）被忽略。
	static Plugin::Permissions permissionsOfNames(const QStringList& names) {
		const QMetaEnum metaEnum = QMetaEnum::fromType<Plugin::Permission>();
		Plugin::Permissions perms;
		for (const QString& name : names) {
			bool ok = false;
			const int value = metaEnum.keyToValue(name.toLatin1().constData(), &ok);
			if (ok) {
				perms |= static_cast<Plugin::Permission>(value);
			}
			else {
				vgWarningF << "Unknown plugin permission name" << name << "found in config, ignored.";
			}
		}
		return perms;
	}

	static QString permissionConfigKey(const QString& pluginID, const char* section) {
		return PermissionConfigRoot + QLatin1Char('.') + pluginID + QLatin1Char('.') + QLatin1String(section);
	}

	// 主程序插件是权限记录的宿主。若它尚未就绪则返回nullptr，调用方应放弃本次落盘。
	static Plugin* permissionStoragePlugin() {
		VIApplication* app = VIApplication::getInstance();
		if (app == nullptr) {
			return nullptr;
		}
		return app->getMainPlugin();
	}

	/*!
		\class Visindigo::General::PluginPermissionRequestHandler
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 插件权限请求的审计处理器接口。

		当插件申请一个尚无结论的权限时，PluginPermissionManager会调用此接口，
		由应用程序（通常是主程序插件）决定允许、仅本次允许或拒绝。

		应用程序应当实现一个自己的处理器（例如弹出对话框询问用户的版本），
		并通过PluginPermissionManager::setPermissionRequestHandler安装。
		未安装处理器时，所有未决的权限申请都会被直接拒绝（fail-closed）。

		\warning 该接口的调用发生在发起申请的线程上，若处理器会阻塞（例如等待用户点击），
		请自行确认调用时机是否允许阻塞。
	*/

	/*!
		\enum Visindigo::General::PluginPermissionRequestHandler::AuditResult
		\value Allow 允许该权限，并永久记录到配置。
		\value AllowThisTime 仅本次允许，不写入配置，下次仍会询问。
		\value Reject 拒绝该权限，并永久记录到配置。
	*/
	PluginPermissionRequestHandler::~PluginPermissionRequestHandler() = default;

	/*!
		\class Visindigo::General::PluginPermissionRequestDefaultHandler
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 以控制台问答方式实现的默认权限处理器。

		以 \c Y 批准、\c YN 仅本次批准、\c N 拒绝，其余输入一律视为拒绝。
		这个处理器需要由应用程序通过
		PluginPermissionManager::setPermissionRequestHandler显式安装才会生效。

		\warning 该处理器通过Console::getLine从标准输入读取用户选择，只适用于命令行程序。
		在GUI程序中用户看不到这条提示且有阻塞风险，请自行实现基于对话框的处理器。
	*/

	/*!
		\since Visindigo 0.17.0
		\a who 发起申请的插件。
		\a reqPerm 被申请的权限。

		return 用户的审计结论。

		该实现只是一个基于控制台的参考实现，输入 \c Y 为永久允许，\c YN 为仅本次允许，
		其余输入一律视为拒绝。它需要被显式安装才会生效，详见
		PluginPermissionRequestDefaultHandler 的文档说明。
	*/
	PluginPermissionRequestHandler::AuditResult PluginPermissionRequestDefaultHandler::onPermissionRequest(Plugin* who, Plugin::Permission reqPerm) {
		vgNotice << "Plugin" << who->getPluginName() << "[" << who->getPluginID() << "] applies for permission"
			<< reqPerm << ", Type Y to approve, YN to approve only this time, or N to reject. [Y/YN/N]";
		const QString input = Visindigo::Utility::Console::getLine().trimmed().toUpper();
		if (input == QStringLiteral("Y")) {
			return Allow;
		}
		if (input == QStringLiteral("YN")) {
			return AllowThisTime;
		}
		return Reject;
	}

	class PluginPermissionManagerPrivate {
		friend class PluginPermissionManager;
	protected:
		static PluginPermissionManager* Instance;
		PluginPermissionRequestHandler* Handler = nullptr;   // 不由本类接管所有权
		QMap<QString, Plugin::Permissions> Allowed;
		QMap<QString, Plugin::Permissions> Rejected;
		QSet<QString> Loaded;                                // 已尝试从配置读取过的插件ID
		void loadRecord(const QString& pluginID);
		void ensureLoaded(const QString& pluginID);
		void saveRecord(const QString& pluginID);
	};
	PluginPermissionManager* PluginPermissionManagerPrivate::Instance = nullptr;

	// 从主程序插件的配置中读取指定插件的权限结论，覆盖内存中的现有内容。
	void PluginPermissionManagerPrivate::loadRecord(const QString& pluginID) {
		Allowed.remove(pluginID);
		Rejected.remove(pluginID);
		Plugin* storage = permissionStoragePlugin();
		if (storage == nullptr) {
			return;
		}
		Visindigo::Utility::JsonConfig* config = storage->getPluginConfig();
		if (config == nullptr) {
			return;
		}
		Allowed.insert(pluginID, permissionsOfNames(config->getStringList(permissionConfigKey(pluginID, "Allowed"))));
		Rejected.insert(pluginID, permissionsOfNames(config->getStringList(permissionConfigKey(pluginID, "Rejected"))));
	}

	// 惰性读取：同一个插件只读一次。主程序插件尚未就绪时不标记为已加载，留给下次调用重试。
	void PluginPermissionManagerPrivate::ensureLoaded(const QString& pluginID) {
		if (Loaded.contains(pluginID)) {
			return;
		}
		if (permissionStoragePlugin() == nullptr) {
			return;
		}
		loadRecord(pluginID);
		Loaded.insert(pluginID);
	}

	// 把指定插件的权限结论写回主程序插件的配置。主程序插件未就绪时只更新内存，不落盘。
	void PluginPermissionManagerPrivate::saveRecord(const QString& pluginID) {
		Plugin* storage = permissionStoragePlugin();
		if (storage == nullptr) {
			vgWarningF << "Cannot persist the permission decision of plugin" << pluginID
				<< "because the main plugin is not ready yet, it will be kept in memory only.";
			return;
		}
		Visindigo::Utility::JsonConfig* config = storage->getPluginConfig();
		if (config == nullptr) {
			return;
		}
		config->setStringList(permissionConfigKey(pluginID, "Allowed"), permissionNamesOf(Allowed.value(pluginID)));
		config->setStringList(permissionConfigKey(pluginID, "Rejected"), permissionNamesOf(Rejected.value(pluginID)));
		storage->savePluginConfig();
	}

	/*!
		\class Visindigo::General::PluginPermissionManager
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 插件权限的审计与记录中心。

		权限记录寄存在主程序插件的配置中，形式为下面两个字符串数组：

		\code
		_visindigo_auto_.Permissions.<插件ID>.Allowed  = ["FileRead", "Network"]
		_visindigo_auto_.Permissions.<插件ID>.Rejected = ["RunAnyCommand"]
		\endcode

		数组元素为Plugin::Permission的枚举键名，便于用户直接阅读、修改或撤销结论。
		被允许或拒绝的结论一经写入即长期生效，且被拒绝的权限不会再次询问用户。

		若主程序插件尚未就绪（例如插件在应用初始化早期就申请权限），
		结论只保留在内存中而不落盘，待主程序插件就绪后的下一次申请再行写入。
	*/

	/*!
		\since Visindigo 0.17.0

		return 权限管理器实例。首次调用时创建，实例在程序生命周期内持续存在。
	*/
	PluginPermissionManager* PluginPermissionManager::getInstance() {
		if (PluginPermissionManagerPrivate::Instance == nullptr) {
			PluginPermissionManagerPrivate::Instance = new PluginPermissionManager;
		}
		return PluginPermissionManagerPrivate::Instance;
	}

	PluginPermissionManager::PluginPermissionManager() {
		d = new PluginPermissionManagerPrivate;
	}

	PluginPermissionManager::~PluginPermissionManager() {
		delete d;
		d = nullptr;
		if (PluginPermissionManagerPrivate::Instance == this) {
			PluginPermissionManagerPrivate::Instance = nullptr;
		}
	}

	/*!
		\since Visindigo 0.17.0
		\a who 发起申请的插件。
		\a perm 被查询的权限。
		\a autoRequest 当该权限尚无结论时，是否允许向用户发起询问。
		return 当前是否拥有该权限。

		只有插件在自身通过setRequiredPermissions声明的权限范围内才会被审计，
		未声明的权限一律返回false，以免插件申请用户完全不了解的权限。

		结论的优先级为：已拒绝 &gt; 已允许 &gt; 询问用户。被拒绝的权限不会再次询问用户，
		如需撤销结论，请修改主程序插件配置中的对应数组。
		当 \a autoRequest 为false时，未决的权限不会打扰用户，直接返回false；
		若为true但应用程序未安装处理器，同样返回false（fail-closed）。
	*/
	bool PluginPermissionManager::hasPermission(Plugin* who, Plugin::Permission perm, bool autoRequest) {
		if (who == nullptr || perm == Plugin::Permission::Unknown) {
			return false;
		}
		if (not who->getRequiredPermissions().testAnyFlag(perm)) {
			vgWarningF << "The permission" << perm << "for inspection is not included in the known required permissions."
				<< "Please set the required permissions in the plugin first, so that users can grasp the details.";
			return false;
		}
		const QString pluginID = who->getPluginID();
		d->ensureLoaded(pluginID);
		if (d->Rejected.value(pluginID).testAnyFlag(perm)) {
			return false;
		}
		if (d->Allowed.value(pluginID).testAnyFlag(perm)) {
			return true;
		}
		if (not autoRequest) {
			return false;
		}
		if (d->Handler == nullptr) {
			vgWarningF << "Plugin" << pluginID << "requests permission" << perm
				<< "but no permission request handler is installed, denied.";
			return false;
		}
		switch (d->Handler->onPermissionRequest(who, perm)) {
		case PluginPermissionRequestHandler::Allow:
			d->Allowed[pluginID] |= perm;
			d->Rejected[pluginID] &= ~Plugin::Permissions(perm);
			d->saveRecord(pluginID);
			return true;
		case PluginPermissionRequestHandler::AllowThisTime:
			return true;
		case PluginPermissionRequestHandler::Reject:
		default:
			d->Rejected[pluginID] |= perm;
			d->Allowed[pluginID] &= ~Plugin::Permissions(perm);
			d->saveRecord(pluginID);
			return false;
		}
	}

	/*!
		\since Visindigo 0.17.0
		\a who 被查询的插件。
		return 该插件当前已被允许的全部权限。
	*/
	Plugin::Permissions PluginPermissionManager::getAllowedPermissions(Plugin* who) {
		if (who == nullptr) {
			return Plugin::Permissions();
		}
		const QString pluginID = who->getPluginID();
		d->ensureLoaded(pluginID);
		return d->Allowed.value(pluginID);
	}

	/*!
		\since Visindigo 0.17.0
		\a who 被查询的插件。
		return 该插件当前已被拒绝的全部权限。
	*/
	Plugin::Permissions PluginPermissionManager::getRejectedPermissions(Plugin* who) {
		if (who == nullptr) {
			return Plugin::Permissions();
		}
		const QString pluginID = who->getPluginID();
		d->ensureLoaded(pluginID);
		return d->Rejected.value(pluginID);
	}

	/*!
		\since Visindigo 0.17.0
		\a who 发起申请的插件。
		\a perm 被申请的权限。
		return 申请是否被批准。

		等价于以 \a autoRequest 为true调用hasPermission，即允许向用户发起询问。
	*/
	bool PluginPermissionManager::requestPermission(Plugin* who, Plugin::Permission perm) {
		return hasPermission(who, perm, true);
	}

	/*!
		\since Visindigo 0.17.0
		\a handler 权限请求处理器，传nullptr表示卸载。

		设置用于询问用户的权限请求处理器。本类不接管 \a handler 的所有权，
		调用方需保证它在被卸载前一直有效。
	*/
	void PluginPermissionManager::setPermissionRequestHandler(PluginPermissionRequestHandler* handler) {
		d->Handler = handler;
	}
}