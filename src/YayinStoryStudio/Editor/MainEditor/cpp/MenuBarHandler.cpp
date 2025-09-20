#include <QtCore/qdir.h>
#include <QtWidgets/qfiledialog.h>
#include <General/YSSProject.h>
#include <Editor/FileServerManager.h>
#include "../MenuBarHandler.h"
#include "../../GlobalValue.h"
#include "../MainWin.h"
#include "../ResourceBrowser.h"
#include "../FileEditorArea.h"
namespace YSS::Editor {
	void Menu_File_FileOptions::newfile() {
		yDebugF << "new File";
		YSS::GlobalValue::getMainWindow()->getResourceBrowser()->openNewFileWindow();
	}

	void Menu_File_FileOptions::open() {
		Visindigo::General::YSSProject* project = GlobalValue::getCurrentProject();
		QDir CurrentDir;
		if (project != nullptr) {
			CurrentDir.setPath(project->getProjectFolder());
		}
		else {
			CurrentDir.setPath(QDir::currentPath());
		}
		QString filePath = QFileDialog::getOpenFileName(
			nullptr,
			"Open File",
			CurrentDir.absolutePath(),
			"All Files (*)");
		if (!filePath.isEmpty()) {
			YSSFSM->openFile(filePath);
		}
	}
	
	void Menu_File_FileOptions::save() {
		yDebugF << "Save File" << YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget()->getFilePath();
		YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget()->saveFile();
	}

	void Menu_File_FileOptions::saveAs() {
		yWarningF << "Save File As not implemented yet";
	}

	void Menu_File_FileOptions::saveAll() {
		yDebugF << "Save All Files";
		YSS::GlobalValue::getMainWindow()->getFileEditorArea()->saveAllFiles();
	}

	void Menu_Edit_EditOptions::undo() {
		yDebugF << "Undo";
		if (YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget() != nullptr) {
			YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget()->undo();
		}
	}
	void Menu_Edit_EditOptions::redo() {
		yDebugF << "Redo";
		if (YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget() != nullptr) {
			YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget()->redo();
		}
	}
	void Menu_Edit_EditOptions::cut() {
		yDebugF << "Cut";
		if (YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget() != nullptr) {
			YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget()->cut();
		}
	}
	void Menu_Edit_EditOptions::copy() {
		yDebugF << "Copy";
		if (YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget() != nullptr) {
			YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget()->copy();
		}
	}
	void Menu_Edit_EditOptions::paste() {
		yDebugF << "Paste";
		if (YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget() != nullptr) {
			YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget()->paste();
		}
	}
	void Menu_Edit_EditOptions::selectAll() {
		yDebugF << "Select All";
		if (YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget() != nullptr) {
			YSS::GlobalValue::getMainWindow()->getFileEditorArea()->getCurrentFileEditWidget()->selectAll();
		}
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