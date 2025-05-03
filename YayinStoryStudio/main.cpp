#include "Editor/MainEditor/MainWin.h"
#include "Editor/GlobalValue.h"
#include <QtWidgets/QApplication>
#include <Utility/ExtTool.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    YSS::GlobalValue g;
	int fontID = QFontDatabase::addApplicationFont(":/compiled/HarmonyOS_Sans_SC_Regular.ttf");
	QString hosFont = QFontDatabase::applicationFontFamilies(fontID).at(0);
	QFont font(hosFont);
	a.setFont(font);

	YSSCore::Utility::ExtTool::registerFileExtension("yssls", "YSS LangServer Library");
    YSS::Editor::MainWin editor;
	editor.show();

    return a.exec();
}
