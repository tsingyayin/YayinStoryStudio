#include "YSSInstaller.h"
#include "Installer/InstallerServer.h"
#include <General/VIApplication.h>
#include <General/Log.h>
#include <General/Version.h>

int main(int argc, char* argv[])
{
    VISetEnv(Visindigo::General::VIApplication::MinimumLoadingTimeMS, 0);
    VISetEnv(Visindigo::General::VIApplication::UseVirtualTerminal, false);
    VISetEnv(Visindigo::General::VIApplication::SaveCommandHistory, false);
    Visindigo::General::VIApplication app(argc, argv, Visindigo::General::VIApplication::WidgetApp, true);
	qApp->setQuitOnLastWindowClosed(false);
	QObject::connect(qApp, &QApplication::lastWindowClosed, []() {
		if (YSS::Installer::InstallerServer::getInstance()->getConnectedClientCount() > 0) {
			vgDebug << "Last window closed, but there are connected clients, not quitting.";
		} else {
			vgDebug << "Last window closed, quitting application.";
			qApp->quit();
		}
		});
    vgInfo << "YSS Installer " << Visindigo::General::Version::getAPIVersion();

    YSSInstaller* mainPlugin = new YSSInstaller();
    app.setMainPlugin(mainPlugin);

    int c = app.start();
    return c;
}
