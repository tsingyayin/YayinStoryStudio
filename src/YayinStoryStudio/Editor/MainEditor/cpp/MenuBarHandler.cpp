#include <QtCore/qdir.h>
#include <QtWidgets/qfiledialog.h>
#include <General/YSSProject.h>
#include <Editor/FileServerManager.h>
#include <Utility/JsonConfig.h>
#include <Editor/DebugServer.h>
#include <Editor/DebugServerManager.h>
#include "../MenuBarHandler.h"
#include "../../GlobalValue.h"
#include "../MainWin.h"
#include "../ResourceBrowser.h"
#include "../FileEditorArea.h"

#define CallYSSDebugServerFunction(functionName, ...) \
	QString debugServerID = YSS::GlobalValue::getCurrentProject()->getProjectConfig()->getString("Project.DebugServerID"); \
	YSSCore::Editor::DebugServer* ds = YSSDSM->getDebugServer(debugServerID); \
	if (ds != nullptr) { \
		ds->functionName(__VA_ARGS__); \
	} \
	else { \
		yErrorF << "Debug server" << debugServerID << "not found!"; \
	}

namespace YSS::Editor {
	void Menu_File_FileOptions::newfile() {
		yDebugF << "new File";
		YSS::GlobalValue::getMainWindow()->getResourceBrowser()->openNewFileWindow();
	}

	void Menu_File_FileOptions::open() {
		YSSCore::General::YSSProject* project = GlobalValue::getCurrentProject();
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

	void Menu_File_ProgramOptions::backToHome() {
		yDebugF << "Back to Home";
		YSS::GlobalValue::getMainWindow()->backToProjectWin();
	}

	void Menu_File_ProgramOptions::exit() {
		yDebugF << "Exit Program";
		YSS::GlobalValue::getMainWindow()->close();
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

	void Menu_Run_RunOptions::run() {
		CallYSSDebugServerFunction(onRun);
	}

	void Menu_Run_RunOptions::debug() {
		CallYSSDebugServerFunction(onDebugStart);
	}

	void Menu_Run_RunOptions::stop() {
		CallYSSDebugServerFunction(onStop);
	}

	void Menu_Run_RunOptions::restart() {
		CallYSSDebugServerFunction(onStop, true);
	}

	void Menu_Run_BuildActions::buildProject() {
		CallYSSDebugServerFunction(onBuild);
	}

	void Menu_Run_BuildActions::cleanProject() {
		CallYSSDebugServerFunction(onClear);
	}

	void Menu_Run_DebugActions::nextStep() {
		CallYSSDebugServerFunction(nextStep);
	}

	void Menu_Run_DebugActions::nextProcess() {
		CallYSSDebugServerFunction(nextProcess);
	}

	void Menu_Run_DebugActions::pause() {
		CallYSSDebugServerFunction(onPause);
	}

	void Menu_Run_DebugActions::continue_() {
		CallYSSDebugServerFunction(onContinue);
	}
	
}