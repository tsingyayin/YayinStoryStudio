#include <QtWidgets/QApplication>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include "Editor/TitlePage/TitlePage.h"
#include "test_main.h"
#include <General/Log.h>
#include <Editor/DebugFailedData.h>
#include <General/Version.h>
#include <QtGui/qwindow.h>
#include <Utility/FileUtility.h>
#include <General/YayinStoryStudio.h>
#include <QtWidgets/qmessagebox.h>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(QFileInfo(a.arguments()[0]).path());
	QStringList prts = YSSCore::Utility::FileUtility::readLines(":/ysscore/compiled/IWillFindU.txt");
	for (auto p : prts) {
		yMessage << p;
	}

	yInfo << "Yayin Story Studio" << YSSCore::General::YayinStoryStudio::getVersion() << "(ABI " << YSSCore::General::YayinStoryStudio::getABIVersion() << ")";
	yNotice << "Launch arguments" << a.arguments();
	yInfo << YSSCore::Utility::FileUtility::getProgramPath();
	yWarning << "YSS Console is showed for debug use. Will be (May be) removed in release version. Do NOT close console directly incase of DATA LOSE !";
	yWarning << "YSS控制台是为了调试目的而显示出来的。将会（可能会）在未来的发行版本中被移除。请勿直接关闭控制台，以避免造成数据丢失。";
	qRegisterMetaType<YSSCore::Editor::DebugFailedData>("YSSCore::Editor::DebugFailedData");
	QMessageBox msgBox;
	msgBox.setWindowTitle("SEA版本警告");
	msgBox.setText("SEA版本（超级早期体验版） 技术预览阶段警告\n\
当前Yayin Story Studio并不完善，功能仍然严重缺失，无法正常作为开发环境使用。\n\
此技术预览版本仅为您展示Yayin Story Studio非常早期的设计思路，以及基本希望拥有的功能的愿景。\n\
敬请期待Yayin Story Studio后续的开发。");
	msgBox.setStandardButtons(QMessageBox::Ok);
	int ret = msgBox.exec();
	YSS::TitlePage::TitlePage titlePage;
	titlePage.setScreen(QApplication::screenAt(QCursor::pos()));
	titlePage.show();

	test_main(a);

	int c = a.exec();
	yNotice << "Program exit in normal way";
	YSSCore::General::LoggerManager::getInstance()->finalSave();
	return c;
}