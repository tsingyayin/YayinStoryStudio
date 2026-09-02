#ifndef YSSInstaller_Installer_VersionManager_h
#define YSSInstaller_Installer_VersionManager_h
#include "Installer/InstallerClientData.h"
#include <QtCore/qobject.h>
namespace YSS::Installer {
	class VersionManagerPrivate;
	class VersionManager :public QObject {
		Q_OBJECT;
	private:
		VersionManager();
	public:
		static VersionManager* getInstance();
		~VersionManager();
	public:
		void recordYSSClient(const InstallerClientData& clientData);
		void updateClientRecord(const InstallerClientData& clientData);
		bool inUpdateProgress();
		QList<InstallerClientData> getClientRecords() const;
		void saveClients();
	private:
		void triggerUpdate(const InstallerClientData& clientData);
		bool isClientValid(const InstallerClientData& client) const;
		VersionManagerPrivate* d;
	};
}
#endif // YSSInstaller_Installer_VersionManager_h
