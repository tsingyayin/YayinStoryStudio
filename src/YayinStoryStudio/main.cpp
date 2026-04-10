#include <QtWidgets/QApplication>
#include "Editor/TitlePage/TitlePage.h"
#include "YayinStoryStudio.h"
#include <General/Log.h>
#include <General/Version.h>
#include <Utility/FileUtility.h>
#include <QtWidgets/qmessagebox.h>
#include <General/VIApplication.h>
#include <General/YSSLogger.h>
#include <General/LoggerMsgHandler.h>

int main(int argc, char* argv[])
{
	//VISetEnv(Visindigo::General::VIApplication::LogFolderPath, Visindigo::Utility::FileUtility::getProgramPath(argv) + "/resource/logs");
	//VISetEnv(Visindigo::General::VIApplication::PluginFolderPath, Visindigo::Utility::FileUtility::getProgramPath(argv) + "/resource/plugins");
	//VISetEnv(Visindigo::General::VIApplication::ThemeFolderPath, Visindigo::Utility::FileUtility::getProgramPath(argv) + "/resource/themes");
	VISetEnv(Visindigo::General::VIApplication::MinimumLoadingTimeMS, 2000);
	VISetEnv(Visindigo::General::VIApplication::UseVirtualTerminal, true);
	VISetEnv(Visindigo::General::VIApplication::SaveCommandHistory, true);
	Visindigo::General::VIApplication app(argc, argv, Visindigo::General::VIApplication::WidgetApp);
	vgDebug<<QSysInfo::productVersion();
	vgDebug << QSysInfo::prettyProductName();
	vgDebug << QSysInfo::kernelType();
	vgDebug << QSysInfo::kernelVersion();

	QStringList prts = Visindigo::Utility::FileUtility::readLines(":/resource/cn.yxgeneral.visindigo/IWillFindU.txt");
	for (auto p : prts) {
		yMessage << p;
	}
	yInfo << "Yayin Story Studio " << Visindigo::General::Version::getAPIVersion();

	QMessageBox::information(nullptr, "SEA版本警告", "SEA版本（超级早期体验版） 技术预览阶段警告\n\
当前Yayin Story Studio并不完善，功能仍然有所缺失，部分交互体验仍然不佳。\n\
此技术预览版本仅为您展示Yayin Story Studio非常早期的设计思路，以及基本希望拥有的功能的愿景。\n\
敬请期待Yayin Story Studio后续的开发。", QMessageBox::Ok);

	YSS::Main* mainPlugin = new YSS::Main();
	mainPlugin->setTestEnable();
	app.setMainPlugin(mainPlugin);
	app.setLoadingMessageHandler(new YSS::TitlePage::TitlePage());

	int c = app.start();
	return c;
}