#include "Editor/MainEditor/MainWin.h"
#include "Editor/GlobalValue.h"
#include <QtWidgets/QApplication>
#include <Utility/ExtTool.h>
#include "Editor/TitlePage/TitlePage.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	YSS::TitlePage::TitlePage titlePage;
	titlePage.show();
    return a.exec();
}
