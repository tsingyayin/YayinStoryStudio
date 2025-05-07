#include <QtWidgets/QApplication>
#include "Editor/TitlePage/TitlePage.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	YSS::TitlePage::TitlePage titlePage;
	titlePage.setScreen(QApplication::screenAt(QCursor::pos()));
	titlePage.show();
    return a.exec();
}
