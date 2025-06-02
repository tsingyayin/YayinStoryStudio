#include <QtWidgets/QApplication>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include "Editor/TitlePage/TitlePage.h"
#include "test_main.h"
#include <General/Log.h>
#include <Editor/DebugFailedData.h>
#include <General/Version.h>
int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	yInfo << "Yayin Story Studio" << YSSCore::General::Version::getYSSVersion();
	//yWarning << "YSS Console is showed for debug use. Will be removed in release version. Do NOT close console directly incase of DATA LOSE !";

	//qRegisterMetaType<YSSCore::Editor::DebugFailedData>("YSSCore::Editor::DebugFailedData");
	QDir::setCurrent(QFileInfo(a.arguments()[0]).path());
	yInfo << a.arguments();
	YSS::TitlePage::TitlePage titlePage;
	titlePage.setScreen(QApplication::screenAt(QCursor::pos()));
	titlePage.show();
#ifdef DEBUG
	test_main(a);
#endif
	int c = a.exec();
	YSSCore::General::LoggerManager::getInstance()->finalSave();
	return c;
}