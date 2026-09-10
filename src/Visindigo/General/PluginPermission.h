#ifndef Visindigo_General_PluginPermission_h
#define Visindigo_General_PluginPermission_h
#include "VICompileMacro.h"
#include "Plugin.h"
namespace Visindigo::General {
	class VisindigoAPI PluginPermissionRequestHandler {
	public:
		enum AuditResult {
			Allow,
			AllowThisTime,
			Reject
		};
		virtual ~PluginPermissionRequestHandler();
		virtual AuditResult onPermissionRequest(Plugin* who, Plugin::Permission reqPerm) = 0;
	};

	class VisindigoAPI PluginPermissionRequestDefaultHandler : public PluginPermissionRequestHandler {
	public:
		virtual AuditResult onPermissionRequest(Plugin* who, Plugin::Permission reqPerm) override;
	};

	class PluginPermissionManagerPrivate;
	class VisindigoAPI PluginPermissionManager :public QObject {
		Q_OBJECT;
	private:
		PluginPermissionManager();
	public:
		~PluginPermissionManager();
		static PluginPermissionManager* getInstance();
	public:
		bool hasPermission(Plugin* who, Plugin::Permission perm, bool autoRequest = false);
		Plugin::Permissions getAllowedPermissions(Plugin* who);
		Plugin::Permissions getRejectedPermissions(Plugin* who);
		bool requestPermission(Plugin* who, Plugin::Permission perm);
		void setPermissionRequestHandler(PluginPermissionRequestHandler* handler);
	private:
		PluginPermissionManagerPrivate* d;
	};
}
#define VIPPM Visindigo::General::PluginPermissionManager::getInstance()
#define VI_HasPerm(who, perm) VIPPM->hasPermission(who, perm, false)
#define VI_HasPermAuto(who, perm) VIPPM->hasPermission(who, perm, true)
#define VI_ReqPerm(who, perm) VIPPM->requestPermission(who, perm)
#endif // Visindigo_General_PluginPermission_h
