#include <QtCore/qdir.h>
#include <QtWidgets/qfiledialog.h>
#include <General/YSSProject.h>
#include <Editor/FileServerManager.h>
#include <Editor/DebugServer.h>
#include <Editor/DebugServerManager.h>
#include "../MenuBarHandler.h"
#include "../../GlobalValue.h"
#include "../MainWin.h"
#include "../ResourceBrowser.h"
#include <General/YSSLogger.h>
#include <Widgets/PluginManageWidget.h>
#include <General/VIApplication.h>
#include <Widgets/Terminal.h>
#include "Editor/MainEditor/StackWidgetArea.h"
#include <QtWidgets/qmessagebox.h>
#include <General/TranslationHost.h>
#include <General/Plugin.h>
#include <Utility/FileUtility.h>

#define CallYSSDebugServerFunction(functionName, ...) \
	QString debugServerID = YSS::GlobalValue::getCurrentProject()->getProjectDebugServerID(); \
	if (debugServerID.isEmpty()) { \
		QMessageBox::warning(YSS::Editor::MainWin::getInstance(),\
			VITR("YSS::editor.debug.notSelected.title"), VITR("YSS::editor.debug.notSelected.message")); \
		vgErrorF << "No debug server set for current project!"; \
		return; \
	} \
	YSSCore::Editor::DebugServer* ds = YSSDSM->getDebugServer(debugServerID); \
	if (ds != nullptr) { \
		ds->functionName(__VA_ARGS__); \
	} \
	else { \
		QMessageBox::warning(YSS::Editor::MainWin::getInstance(),\
			VITR("YSS::editor.debug.notFound.title"), VITR("YSS::editor.debug.notFound.message").arg(debugServerID)); \
		vgErrorF << "Debug server" << debugServerID << "not found!"; \
	}

namespace YSS::Editor {
	void Menu_File_FileOptions::newfile() {
		yDebugF << "new File";
		YSS::Editor::MainWin::getInstance()->getResourceBrowser()->openNewFileWindow();
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
		yDebugF << "Save File" << YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget()->getFilePath();
		YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget()->saveFile();
	}

	void Menu_File_FileOptions::saveAs() {
		yWarningF << "Save File As not implemented yet";
	}

	void Menu_File_FileOptions::saveAll() {
		yDebugF << "Save All Files";
		YSS::Editor::MainWin::getInstance()->saveAll();
	}

	void Menu_File_FileOptions::projectConfig() {
		QString ysspFilePath = YSS::GlobalValue::getCurrentProject()->getProjectConfigPath();
		YSSFSM->openFile(ysspFilePath);
	}

	void Menu_File_ProgramOptions::backToHome() {
		yDebugF << "Back to Home";
		YSS::Editor::MainWin::getInstance()->backToProjectWin();
	}

	void Menu_File_ProgramOptions::terminal() {
		auto terminal = VIApp->getVirtualTerminal();
		if (terminal) { 
			terminal->show();
		}
	}
	void Menu_File_ProgramOptions::pluginAndPreferences() {
		yDebugF << "Plugin and Preference";
		auto widget = new Visindigo::Widgets::PluginManageWidget();
		widget->setAttribute(Qt::WA_DeleteOnClose);
		widget->setWindowModality(Qt::ApplicationModal);
		widget->setWindowFlags(widget->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
		widget->show();
	}

	void Menu_File_ProgramOptions::about() {
		yDebugF << "About";
		QMessageBox::about(YSS::Editor::MainWin::getInstance(),
			VITR("YSS::editor.about.title"),
			VITR("YSS::editor.about.message").arg(VIApp->getMainPlugin()->getPluginVersion().toString()));
	}

	void Menu_File_ProgramOptions::documentation() {
		yDebugF << "Documentation";
		Visindigo::Utility::FileUtility::openBrowser("http://prts.site");
	}

	void Menu_File_ProgramOptions::exit() {
		yDebugF << "Exit Program";
		YSS::Editor::MainWin::getInstance()->close();
	}

	void Menu_Edit_EditOptions::undo() {
		yDebugF << "Undo";
		if (YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget() != nullptr) {
			YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget()->undo();
		}
	}
	void Menu_Edit_EditOptions::redo() {
		yDebugF << "Redo";
		if (YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget() != nullptr) {
			YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget()->redo();
		}
	}
	void Menu_Edit_EditOptions::cut() {
		yDebugF << "Cut";
		if (YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget() != nullptr) {
			YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget()->cut();
		}
	}
	void Menu_Edit_EditOptions::copy() {
		yDebugF << "Copy";
		if (YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget() != nullptr) {
			YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget()->copy();
		}
	}
	void Menu_Edit_EditOptions::paste() {
		yDebugF << "Paste";
		if (YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget() != nullptr) {
			YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget()->paste();
		}
	}
	void Menu_Edit_EditOptions::selectAll() {
		yDebugF << "Select All";
		if (YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget() != nullptr) {
			YSS::Editor::MainWin::getInstance()->getStackWidgetArea()->getCurrentWidget()->selectAll();
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

	void Menu_Run_RunOptions::aboutToShow() {
		
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