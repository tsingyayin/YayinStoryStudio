#include <QtCore/qdir.h>
#include <General/Log.h>
#include <General/PluginManager.h>
#include <General/TranslationHost.h>
#include <General/Version.h>
#include <General/VIApplication.h>
#include <Utility/FileOperation.h>
#include <Utility/FileUtility.h>
#include <Widgets/ConfigWidget.h>
#include "Installer/InstallerServer.h"
#include "Installer/LocalUpdateWizard.h"
#include "Installer/TrayIcon.h"
#include "YSSInstaller.h"
YSSInstallerTranslator::YSSInstallerTranslator(Visindigo::General::Plugin* parent) :
	Visindigo::General::Translator(parent, "YSSInstaller")
{
	setDefaultLang(zh_CN);
	addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/zh_CN.json");
	addLangFilePath(zh_TW, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/zh_TW.json");
	addLangFilePath(en, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/en.json");
	addLangFilePath(ja, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/ja.json");
	addLangFilePath(jp_less_loanword, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/jp_less_loanword.json");
	addLangFilePath(ko, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/ko.json");
	addLangFilePath(ru, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/ru.json");
	addLangFilePath(de, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/de.json");
	addLangFilePath(fr, ":/resource/cn.yxgeneral.yayinstorystudio.installer/i18n/fr.json");
}

class YSSInstallerPrivate {
    friend class YSSInstaller;
protected:
    Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
    static YSSInstaller* Instance;
};
YSSInstaller* YSSInstallerPrivate::Instance = nullptr;

YSSInstaller::YSSInstaller() : Visindigo::General::Plugin("cn.yxgeneral.yayinstorystudio.installer") {
    d = new YSSInstallerPrivate;
    YSSInstallerPrivate::Instance = this;
    setPluginVersion(getPluginAPIVersion());
    setPluginName("YSS Installer");
    setPluginAuthor({ "Tsing Yayin" });
}

YSSInstaller::~YSSInstaller() {
    delete d;
}

void YSSInstaller::onPluginEnable() {
	// Migrate the config of the legacy plugin ID to the current one, if it exists.
	const QString legacyPluginID = "cn.yxgeneral.yss_installer";
	const QDir legacyFolder = VIPLM->getPluginFolder(legacyPluginID, Visindigo::General::Plugin::LoadType::MainPlugin);
	const QString legacyConfigPath = legacyFolder.filePath("config.json");
	if (Visindigo::Utility::FileUtility::isFileExist(legacyConfigPath)) {
		const QString configPath = getPluginFolder().filePath("config.json");
		const Visindigo::Utility::FileOperation::ErrorCode copyResult =
			Visindigo::Utility::FileOperation::copyFile(legacyConfigPath, configPath, true, true);
		if (copyResult != Visindigo::Utility::FileOperation::Success) {
			vgWarningF << "Failed to migrate config from legacy plugin ID" << legacyPluginID
				<< ", error:" << Visindigo::Utility::FileOperation::errorCodeName(copyResult);
		}
		else {
			reloadPluginConfig();
			// The migration is one-shot: remove the legacy folder so it won't be migrated again.
			const Visindigo::Utility::FileOperation::ErrorCode removeResult =
				Visindigo::Utility::FileOperation::deleteDir(legacyFolder.absolutePath(), false);
			if (removeResult != Visindigo::Utility::FileOperation::Success) {
				vgWarningF << "Failed to remove legacy plugin folder" << legacyFolder.absolutePath()
					<< ", error:" << Visindigo::Utility::FileOperation::errorCodeName(removeResult);
			}
		}
	}
    VITRH->setLangID(Visindigo::General::Translator::zh_CN);
	registerPluginModule(new YSSInstallerTranslator(this));
    YSS::Installer::TrayIcon* trayIcon = new YSS::Installer::TrayIcon();
    trayIcon->show();
    YSS::Installer::InstallerServer* installerServer = new YSS::Installer::InstallerServer();
    connect(installerServer, &YSS::Installer::InstallerServer::serverEstablished, this, []() {
        vgDebug << "Installer server established.";
        });
    connect(installerServer, &YSS::Installer::InstallerServer::installerHasLaunched, this, []() {
        vgDebug << "Installer has launched.";
        // if in event loop
        exit(0);
        });
	connect(installerServer, &YSS::Installer::InstallerServer::clientConnected, this, [](QLocalSocket* client) {
        vgDebug << "Client connected to installer server:" << client;
		});
    installerServer->launchServer();
}

void YSSInstaller::onApplicationInit() {
    // Application init: show installer UI here
}

void YSSInstaller::onPluginDisable() {
    // Plugin disabled actions
}

void YSSInstaller::onTest() {
    // Test actions
}

QWidget* YSSInstaller::getConfigWidget() {
    return d->ConfigWidget;
}

YSSInstaller* YSSInstaller::getInstance() {
    return YSSInstallerPrivate::Instance;
}

