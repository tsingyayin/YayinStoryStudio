#include "Editor/MainEditor/MainWin.h"
#include "Editor/GlobalValue.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    YSS::GlobalValue g;
   /* YSS::Editor::TextEdit w;
    w.show();
    w.openFile("D:\\ASE\\ASE 2.04.23.0\\Arknights_StoryEditor_Data\\StreamingAssets\\Stories\\百步-1.astory");*/
    YSS::Editor::MainWin editor;
	editor.show();
 //   YSS::Editor::ResourceBrowser browser;
	//browser.show();
    return a.exec();
}
