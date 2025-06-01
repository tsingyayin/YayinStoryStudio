#include <QtWidgets/QApplication>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include "Editor/TitlePage/TitlePage.h"
#include "test_main.h"
#include <General/Log.h>
int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(QFileInfo(a.arguments()[0]).path());
	yDebug << a.arguments();
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