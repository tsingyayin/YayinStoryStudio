#include "YSSInstaller.h"
#include <General/VIApplication.h>
#include <General/Log.h>
#include <General/Version.h>

int main(int argc, char* argv[])
{
    VISetEnv(Visindigo::General::VIApplication::MinimumLoadingTimeMS, 1500);
    VISetEnv(Visindigo::General::VIApplication::UseVirtualTerminal, true);
    VISetEnv(Visindigo::General::VIApplication::SaveCommandHistory, true);
    Visindigo::General::VIApplication app(argc, argv, Visindigo::General::VIApplication::WidgetApp, true);

    vgInfo << "YSS Installer " << Visindigo::General::Version::getAPIVersion();

    YSSInstaller* mainPlugin = new YSSInstaller();
    app.setMainPlugin(mainPlugin);

    int c = app.start();
    return c;
}
