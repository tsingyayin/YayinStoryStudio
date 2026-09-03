#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/Version.h>
#include <General/VIApplication.h>
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
	addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.yss_installer/i18n/zh_CN.json");
	addLangFilePath(zh_TW, ":/resource/cn.yxgeneral.yss_installer/i18n/zh_TW.json");
	addLangFilePath(en, ":/resource/cn.yxgeneral.yss_installer/i18n/en.json");
	addLangFilePath(ja, ":/resource/cn.yxgeneral.yss_installer/i18n/ja.json");
	addLangFilePath(jp_less_loanword, ":/resource/cn.yxgeneral.yss_installer/i18n/jp_less_loanword.json");
	addLangFilePath(ko, ":/resource/cn.yxgeneral.yss_installer/i18n/ko.json");
	addLangFilePath(ru, ":/resource/cn.yxgeneral.yss_installer/i18n/ru.json");
	addLangFilePath(de, ":/resource/cn.yxgeneral.yss_installer/i18n/de.json");
	addLangFilePath(fr, ":/resource/cn.yxgeneral.yss_installer/i18n/fr.json");
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

