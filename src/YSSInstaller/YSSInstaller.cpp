#include "YSSInstaller.h"
#include <General/VIApplication.h>
#include <General/Log.h>
#include <General/Version.h>
#include <Widgets/ConfigWidget.h>
#include <Utility/FileUtility.h>
#include "Installer/TrayIcon.h"
#include "Installer/InstallerServer.h"
class YSSInstallerPrivate {
    friend class YSSInstaller;
protected:
    Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
    static YSSInstaller* Instance;
};
YSSInstaller* YSSInstallerPrivate::Instance = nullptr;

YSSInstaller::YSSInstaller() {
    d = new YSSInstallerPrivate;
    YSSInstallerPrivate::Instance = this;
    setPluginVersion(getPluginAPIVersion());
    setPluginID("cn.yxgeneral.yss_installer");
    setPluginName("YSS Installer");
    setPluginAuthor({ "Tsing Yayin" });
}

YSSInstaller::~YSSInstaller() {
    delete d;
}

void YSSInstaller::onPluginEnable() {
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

