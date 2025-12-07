#include <QtWidgets/QApplication>
#include "Editor/TitlePage/TitlePage.h"
#include "YayinStoryStudio.h"
#include <General/Log.h>
#include <Editor/DebugFailedData.h>
#include <General/Version.h>
#include <Utility/FileUtility.h>
#include <QtWidgets/qmessagebox.h>
#include <General/VIApplication.h>

int main(int argc, char* argv[])
{
	VIAppStatic::setEnvConfig(Visindigo::General::VIApplication::LogFolderPath, Visindigo::Utility::FileUtility::getProgramPath(argv) + "/resource/logs");
	VIAppStatic::setEnvConfig(Visindigo::General::VIApplication::PluginFolderPath, Visindigo::Utility::FileUtility::getProgramPath(argv) + "/resource/plugins");
	VIAppStatic::setEnvConfig(Visindigo::General::VIApplication::MinimumLoadingTimeMS, 2000);
	Visindigo::General::VIApplication app(argc, argv, Visindigo::General::VIApplication::WidgetApp);
	

	QStringList prts = Visindigo::Utility::FileUtility::readLines(":/Visindigo/compiled/IWillFindU.txt");
	for (auto p : prts) {
		yMessage << p;
	}

	yInfo << "Yayin Story Studio " << Visindigo::General::Version::getAPIVersion();
	yWarning << "YSS Console is showed for debug use. Will be (May be) removed in release version. Do NOT close console directly incase of DATA LOSE !";
	yWarning << "YSS控制台是为了调试目的而显示出来的。将会（可能会）在未来的发行版本中被移除。请勿直接关闭控制台，以避免造成数据丢失。";

	//qRegisterMetaType<YSSCore::Editor::DebugFailedData>("YSSCore::Editor::DebugFailedData");

	QMessageBox::information(nullptr, "SEA版本警告", "SEA版本（超级早期体验版） 技术预览阶段警告\n\
当前Yayin Story Studio并不完善，功能仍然严重缺失，无法正常作为开发环境使用。\n\
此技术预览版本仅为您展示Yayin Story Studio非常早期的设计思路，以及基本希望拥有的功能的愿景。\n\
敬请期待Yayin Story Studio后续的开发。", QMessageBox::Ok);

	app.setMainPlugin(new YSS::Main());
	app.setLoadingMessageHandler(new YSS::TitlePage::TitlePage());
	int c = app.start();
	return c;
}