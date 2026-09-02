#ifndef YSSInstaller_YSSInstaller_h
#define YSSInstaller_YSSInstaller_h
#include <General/Package.h>
#include <QtWidgets/qwidget.h>
#include <General/Translator.h>

class YSSInstallerTranslator : public Visindigo::General::Translator
{
public:
	YSSInstallerTranslator(Visindigo::General::Plugin* parent);
};
class YSSInstallerPrivate;
class YSSInstaller : public Visindigo::General::Package
{
    Q_OBJECT;
public:
    YSSInstaller();
    virtual ~YSSInstaller();
public:
    virtual void onPluginEnable() override;
    virtual void onApplicationInit() override;
    virtual void onPluginDisable() override;
    virtual void onTest() override;
    virtual QWidget* getConfigWidget() override;
public:
    static YSSInstaller* getInstance();
private:
    YSSInstallerPrivate* d;
};

#define YSSInstallerApp YSSInstaller::getInstance()

#endif // YSSInstaller_YSSInstaller_h
