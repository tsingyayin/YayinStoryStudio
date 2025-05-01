#include "Editor/TextEdit.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    YSE::Editor::TextEdit w;
    w.show();
    return a.exec();
}
