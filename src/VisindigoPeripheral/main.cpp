#include "VisindigoPeripheral.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    VisindigoPeripheral window;
    window.show();
    return app.exec();
}
