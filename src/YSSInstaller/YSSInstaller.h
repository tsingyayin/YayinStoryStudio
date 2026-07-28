#pragma once

#include <General/Package.h>
#include <QtWidgets/qwidget.h>

class YSSInstallerPrivate;

class YSSInstaller : public Visindigo::General::Package
{
    Q_OBJECT;

public:
    YSSInstaller();
    virtual ~YSSInstaller();

    virtual void onPluginEnable() override;
    virtual void onApplicationInit() override;
    virtual void onPluginDisable() override;
    virtual void onTest() override;
    virtual QWidget* getConfigWidget() override;

    static YSSInstaller* getInstance();

private:
    YSSInstallerPrivate* d;
};

#define YSSInstallerApp YSSInstaller::getInstance()

