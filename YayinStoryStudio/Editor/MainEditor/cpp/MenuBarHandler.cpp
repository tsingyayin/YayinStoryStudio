#include "../MenuBarHandler.h"
#include "../../GlobalValue.h"
#include "../MainWin.h"
#include "../ResourceBrowser.h"
namespace YSS::Editor {
	void Menu_File_FileOptions::newfile() {
		yDebugF << "new File";
		YSS::GlobalValue::getMainWindow()->getResourceBrowser()->openNewFileWindow();
	}

	void Menu_File_ProgramOptions::backToHome() {
		yDebugF << "Back to Home";
		YSS::GlobalValue::getMainWindow()->backToProjectWin();
	}

	void Menu_File_ProgramOptions::exit() {
		yDebugF << "Exit Program";
		YSS::GlobalValue::getMainWindow()->close();
	}
}