#include "YSSInstaller.h"
#include <General/VIApplication.h>
#include <General/Log.h>
#include <General/Version.h>
#include <Widgets/ConfigWidget.h>
#include <Utility/FileUtility.h>

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
    setPluginID("cn.yxgeneral.yayinstorystudio.installer");
    setPluginName("YSS Installer");
    setPluginAuthor({ "Tsing Yayin" });
}

YSSInstaller::~YSSInstaller() {
    delete d;
}

void YSSInstaller::onPluginEnable() {
    // Installer plugin enabled actions
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

