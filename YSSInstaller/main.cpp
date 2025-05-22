#include "YSSInstaller.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    YSSInstaller w;
    w.show();
    return a.exec();
}
