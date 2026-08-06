#include "Installer/VersionManager.h"
#include <General/VIApplication.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
#include <General/Plugin.h>
#include "Installer/LocalUpdateWizard.h"
#include "Installer/InstallerServer.h"
#include <QtCore/qfileinfo.h>
#include <QtCore/qdatetime.h>
#include <General/Log.h>
namespace YSS::Installer {
	class VersionManagerPrivate {
		friend class VersionManager;
	protected:
		QList<InstallerClientData> clientRecords;
		Visindigo::General::Version newestVersion;
		LocalUpdateWizard* updateWizard = nullptr;
		static VersionManager* Instance;
	};
	VersionManager* VersionManagerPrivate::Instance = nullptr;

	VersionManager* VersionManager::getInstance() {
		if (VersionManagerPrivate::Instance == nullptr) {
			VersionManagerPrivate::Instance = new VersionManager();
		}
		return VersionManagerPrivate::Instance;
	}

	VersionManager::VersionManager() {
		d = new VersionManagerPrivate();
		Visindigo::Utility::JsonConfig* config = VIApp->getMainPlugin()->getPluginConfig();
		if (config->contains("client_records")) {
			QList<Visindigo::Utility::JsonConfig> records = config->getArray("client_records");
			for (const Visindigo::Utility::JsonConfig& record : records) {
				InstallerClientData clientData;
				clientData.setProgramPath(record.getString("program_path"));
				clientData.setProgramVersion(record.getString("program_version"));
				clientData.setAutoUpdateEnabled(record.getBool("auto_update_enabled"));
				
				// 有效性检测：可执行文件不存在则跳过该记录。
				if (!isClientValid(clientData)) {
					continue;
				}
				d->clientRecords.append(clientData);
				if (Visindigo::General::Version(clientData.getProgramVersion()) > d->newestVersion) {
					d->newestVersion = Visindigo::General::Version(clientData.getProgramVersion());
				}
			}
		}
	}

	VersionManager::~VersionManager() {
		delete d;
	}

	void VersionManager::recordYSSClient(const InstallerClientData& clientData) {
		if (d->clientRecords.isEmpty()) {
			d->newestVersion = Visindigo::General::Version(clientData.getProgramVersion());
			d->clientRecords.append(clientData);
			saveClients();
			return;
		}
		for (auto& client : d->clientRecords) {
			if (client.getProgramPath() == clientData.getProgramPath()) {
				client = clientData;
				if (Visindigo::General::Version(clientData.getProgramVersion()) > d->newestVersion) {
					d->newestVersion = Visindigo::General::Version(clientData.getProgramVersion());
				}
				saveClients();
				return;
			}
		}
		// new client that never recorded before.
		if (Visindigo::General::Version(clientData.getProgramVersion()) > d->newestVersion) {
			triggerUpdate(clientData);
			for (auto& client : d->clientRecords) {
				if (client.getAutoUpdateEnabled()) {
					// auto update logic is not used yet.
				}
			}
		}
		else {
			d->clientRecords.append(clientData); // new client but old version, just record it.
		}
	}

	void VersionManager::updateClientRecord(const InstallerClientData& clientData) {
		for (auto& client : d->clientRecords) {
			if (client.getProgramPath() == clientData.getProgramPath()) {
				client = clientData;
				if (Visindigo::General::Version(clientData.getProgramVersion()) > d->newestVersion) {
					d->newestVersion = Visindigo::General::Version(clientData.getProgramVersion());
				}
				saveClients();
				return;
			}
		}
	}

	bool VersionManager::inUpdateProgress() {
		return d->updateWizard != nullptr;
	}

	QList<InstallerClientData> VersionManager::getClientRecords() const {
		return d->clientRecords;
	}

	void VersionManager::saveClients() {
		Visindigo::Utility::JsonConfig* config = VIApp->getMainPlugin()->getPluginConfig();
		QList<Visindigo::Utility::JsonConfig> records;
		for (const InstallerClientData& client : d->clientRecords) {
			vgDebug << client.getProgramPath();
			if (!isClientValid(client)) {
				continue;
			}
			Visindigo::Utility::JsonConfig record;
			record.setString("program_path", client.getProgramPath());
			record.setString("program_version", client.getProgramVersion());
			record.setBool("auto_update_enabled", client.getAutoUpdateEnabled());
			records.append(record);
		}
		config->setArray("client_records", records);
		VIApp->getMainPlugin()->savePluginConfig();
	}

	bool VersionManager::isClientValid(const InstallerClientData& client) const {
		return Visindigo::Utility::FileUtility::isFileExist(client.getProgramPath());
	}

	void VersionManager::triggerUpdate(const InstallerClientData& clientData) {
		QString selfPath = VIApp->getProgramPath();
		QDateTime selfDate = Visindigo::Utility::FileUtility::getFileModifyTime(selfPath);
		QFileInfo clientExeInfo(clientData.getProgramPath());
		QString clientInstallerPath = clientExeInfo.absolutePath() + "/YSSInstaller.exe";
		QDateTime clientInstallerDate = Visindigo::Utility::FileUtility::getFileModifyTime(clientInstallerPath);
		if (clientInstallerDate.isValid() && selfDate.isValid() && clientInstallerDate > selfDate) {
			vgNotice << "A newer YSSInstaller carried by client is available, requesting update.";
			InstallerServer::getInstance()->sendUpdateYSSInstallerRequest(clientData);
			return; // YSSInstaller should be updated first.
		}

		if (d->updateWizard == nullptr) {
			d->updateWizard = new LocalUpdateWizard(clientData);
			d->updateWizard->setAttribute(Qt::WA_DeleteOnClose);
			connect(d->updateWizard, &LocalUpdateWizard::destroyed, this, [this]() {
				d->updateWizard = nullptr;
				});
			connect(d->updateWizard, &LocalUpdateWizard::asIndependent, this, [this](const InstallerClientData& clientData) {
				bool found = false;
				d->newestVersion = Visindigo::General::Version(clientData.getProgramVersion());
				for (auto& client : d->clientRecords) {
					if (client.getProgramPath() == clientData.getProgramPath()) {
						client = clientData;
						found = true;
						break;
					}
				}
				if (not found) {
					d->clientRecords.append(clientData);
				}		
				saveClients();
			});
			d->updateWizard->show();
		}
		else {
			vgError << "Update wizard is already running, cannot trigger another update.";
		}
	}
}