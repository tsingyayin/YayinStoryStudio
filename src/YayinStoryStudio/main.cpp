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

	QMessageBox::information(nullptr, "SEA TP 技术预览版本警告", 
R"(警告！
您当前正在使用技术预览版本！

技术预览版本仅供用户体验和测试新功能，并帮助我们对正式版程序进行完善。技术预览版本很可能非常不稳定,可能会导致数据丢失或其他严重问题。请勿在重要项目或生产环境中使用技术预览版本。

敬请期待Yayin Story Studio后续的开发。)", QMessageBox::Ok);

	YSS::Main* mainPlugin = new YSS::Main();
	mainPlugin->setTestEnable();
	app.setMainPlugin(mainPlugin);
	app.setLoadingMessageHandler(new YSS::TitlePage::TitlePage());

	int c = app.start();
	return c;
}