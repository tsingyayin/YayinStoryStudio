#include "Editor/MainEditor/MainWinMenu.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/MainEditor/ResourceBrowser.h"
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include "Editor/MainEditor/DebugServerRouter.h"
#include <Editor/FileEditWidget.h>
#include "Editor/GlobalValue.h"
#include <General/YSSProject.h>
#include <General/TranslationHost.h>
#include <QtWidgets/qlabel.h>
#include <Widgets/BorderLabel.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qboxlayout.h>
#include <Utility/FileUtility.h>
#include <General/VIApplication.h>
#include <Widgets/Terminal.h>
#include <Widgets/PluginManageWidget.h>
#include <QtWidgets/qmessagebox.h>
#include <General/Plugin.h>
#include <Editor/TextEdit.h>
#include <Editor/ToolWidgetManager.h>
#include <Utility/QtSSHelper.h>
namespace YSS::Editor {
	class MainWinMenuPrivate {
		friend class MainWinMenu;
	protected:
		MainWinMenu* q;
		MainWin* Parent;
		QHBoxLayout* Layout;
		Visindigo::Widgets::BorderLabel* ProjectNameLabel;
		QMenuBar* MenuBar;
		QMenu* FileMenu;
		QAction* File_File_New;
		QAction* File_File_Open;
		QAction* File_File_Save;
		QAction* File_File_SaveAll;
		QAction* File_File_SaveAs;
		QAction* File_File_ProjectConfig;
		QAction* File_File_ShowInExplorer;
		QAction* File_Program_BackToHome;
		QAction* File_Program_Terminal;
		QAction* File_Program_PluginAndPreferences;
		QAction* File_Program_About;
		QAction* File_Program_Documentation;
		QAction* File_Program_Exit;

		QMenu* EditMenu;
		QAction* Edit_Undo;
		QAction* Edit_Redo;
		QAction* Edit_Cut;
		QAction* Edit_Copy;
		QAction* Edit_Paste;
		QAction* Edit_SelectAll;
		QAction* Edit_FindAndReplace;

		QMenu* RunMenu;
		QAction* Run_Run_Run;
		QAction* Run_Run_Debug;
		QAction* Run_Run_Stop;
		QAction* Run_Run_Restart;
		QAction* Run_Build_Build;
		QAction* Run_Build_Rebuild;
		QAction* Run_Build_Clean;
		QAction* Run_Action_StepInto;
		QAction* Run_Action_StepOver;
		QAction* Run_Action_Suspend;
		QAction* Run_Action_Resume;

		QMenu* ViewMenu;
		QAction* View_ResourceBrowser;
		QAction* View_MessageViewer;
		QMenu* View_PluginTools;
		QMap<QString, QString> PluginToolsMap;
		QMap<QAction*, QString> PluginToolsActionIDMap;

		void initFileMenu() {
			FileMenu = MenuBar->addMenu(VITR("YSS::menu.file.title"));
			FileMenu->setObjectName("file");
			File_File_New = FileMenu->addAction(VITR("YSS::menu.file.new"));
			File_File_New->setObjectName("new");
			File_File_Open = FileMenu->addAction(VITR("YSS::menu.file.open"));
			File_File_Open->setObjectName("open");
			File_File_Save = FileMenu->addAction(VITR("YSS::menu.file.save"));
			File_File_Save->setObjectName("save");
			File_File_SaveAll = FileMenu->addAction(VITR("YSS::menu.file.saveAll"));
			File_File_SaveAll->setObjectName("saveAll");
			File_File_SaveAs = FileMenu->addAction(VITR("YSS::menu.file.saveAs"));
			File_File_SaveAs->setObjectName("saveAs");
			FileMenu->addSeparator();
			File_File_ProjectConfig = FileMenu->addAction(VITR("YSS::menu.file.projectConfig"));
			File_File_ProjectConfig->setObjectName("projectConfig");
			File_File_ShowInExplorer = FileMenu->addAction(VITR("YSS::menu.file.showInExplorer"));
			File_File_ShowInExplorer->setObjectName("showInExplorer");
			FileMenu->addSeparator();
			File_Program_BackToHome = FileMenu->addAction(VITR("YSS::menu.file.backToHome"));
			File_Program_BackToHome->setObjectName("backToHome");
			File_Program_Terminal = FileMenu->addAction(VITR("YSS::menu.file.terminal"));
			File_Program_Terminal->setObjectName("terminal");
			File_Program_PluginAndPreferences = FileMenu->addAction(VITR("YSS::menu.file.pluginAndPreferences"));
			File_Program_PluginAndPreferences->setObjectName("pluginAndPreferences");
			File_Program_About = FileMenu->addAction(VITR("YSS::menu.file.about"));
			File_Program_About->setObjectName("about");
			File_Program_Documentation = FileMenu->addAction(VITR("YSS::menu.file.documentation"));
			File_Program_Documentation->setObjectName("documentation");
			FileMenu->addSeparator();
			File_Program_Exit = FileMenu->addAction(VITR("YSS::menu.file.exit"));
			File_Program_Exit->setObjectName("exit");

			QObject::connect(File_File_New, &QAction::triggered, q, &MainWinMenu::file_file_new);
			QObject::connect(File_File_Open, &QAction::triggered, q, &MainWinMenu::file_file_open);
			QObject::connect(File_File_Save, &QAction::triggered, q, &MainWinMenu::file_file_save);
			QObject::connect(File_File_SaveAll, &QAction::triggered, q, &MainWinMenu::file_file_saveAll);
			QObject::connect(File_File_SaveAs, &QAction::triggered, q, &MainWinMenu::file_file_saveAs);
			QObject::connect(File_File_ProjectConfig, &QAction::triggered, q, &MainWinMenu::file_file_projectConfig);
			QObject::connect(File_File_ShowInExplorer, &QAction::triggered, q, &MainWinMenu::file_file_showInExplorer);
			QObject::connect(File_Program_BackToHome, &QAction::triggered, q, &MainWinMenu::file_program_backToHome);
			QObject::connect(File_Program_Terminal, &QAction::triggered, q, &MainWinMenu::file_program_terminal);
			QObject::connect(File_Program_PluginAndPreferences, &QAction::triggered, q, &MainWinMenu::file_program_pluginAndPreferences);
			QObject::connect(File_Program_About, &QAction::triggered, q, &MainWinMenu::file_program_about);
			QObject::connect(File_Program_Documentation, &QAction::triggered, q, &MainWinMenu::file_program_documentation);
			QObject::connect(File_Program_Exit, &QAction::triggered, q, &MainWinMenu::file_program_exit);
		}

		void initEditMenu() {
			EditMenu = MenuBar->addMenu(VITR("YSS::menu.edit.title"));
			EditMenu->setObjectName("edit");
			Edit_Undo = EditMenu->addAction(VITR("YSS::menu.edit.undo"));
			Edit_Undo->setObjectName("undo");
			Edit_Redo = EditMenu->addAction(VITR("YSS::menu.edit.redo"));
			Edit_Redo->setObjectName("redo");
			EditMenu->addSeparator();
			Edit_Cut = EditMenu->addAction(VITR("YSS::menu.edit.cut"));
			Edit_Cut->setObjectName("cut");
			Edit_Copy = EditMenu->addAction(VITR("YSS::menu.edit.copy"));
			Edit_Copy->setObjectName("copy");
			Edit_Paste = EditMenu->addAction(VITR("YSS::menu.edit.paste"));
			Edit_Paste->setObjectName("paste");
			Edit_SelectAll = EditMenu->addAction(VITR("YSS::menu.edit.selectAll"));
			Edit_SelectAll->setObjectName("selectAll");
			EditMenu->addSeparator();
			Edit_FindAndReplace = EditMenu->addAction(VITR("YSS::menu.edit.findAndReplace"));
			Edit_FindAndReplace->setObjectName("findAndReplace");

			QObject::connect(Edit_Undo, &QAction::triggered, q, &MainWinMenu::edit_undo);
			QObject::connect(Edit_Redo, &QAction::triggered, q, &MainWinMenu::edit_redo);
			QObject::connect(Edit_Cut, &QAction::triggered, q, &MainWinMenu::edit_cut);
			QObject::connect(Edit_Copy, &QAction::triggered, q, &MainWinMenu::edit_copy);
			QObject::connect(Edit_Paste, &QAction::triggered, q, &MainWinMenu::edit_paste);
			QObject::connect(Edit_SelectAll, &QAction::triggered, q, &MainWinMenu::edit_selectAll);
			QObject::connect(Edit_FindAndReplace, &QAction::triggered, q, &MainWinMenu::edit_findAndReplace);
		}

		void initRunMenu() {
			RunMenu = MenuBar->addMenu(VITR("YSS::menu.run.title"));
			RunMenu->setObjectName("run");
			Run_Run_Run = RunMenu->addAction(VITR("YSS::menu.run.run"));
			Run_Run_Run->setObjectName("run");
			Run_Run_Debug = RunMenu->addAction(VITR("YSS::menu.run.debug"));
			Run_Run_Debug->setObjectName("debug");
			Run_Run_Stop = RunMenu->addAction(VITR("YSS::menu.run.stop"));
			Run_Run_Stop->setObjectName("stop");
			Run_Run_Restart = RunMenu->addAction(VITR("YSS::menu.run.restart"));
			Run_Run_Restart->setObjectName("restart");
			RunMenu->addSeparator();
			Run_Build_Build = RunMenu->addAction(VITR("YSS::menu.run.build"));
			Run_Build_Build->setObjectName("build");
			Run_Build_Rebuild = RunMenu->addAction(VITR("YSS::menu.run.rebuild"));
			Run_Build_Rebuild->setObjectName("rebuild");
			Run_Build_Clean = RunMenu->addAction(VITR("YSS::menu.run.clean"));
			Run_Build_Clean->setObjectName("clean");
			RunMenu->addSeparator();
			Run_Action_StepInto = RunMenu->addAction(VITR("YSS::menu.run.stepInto"));
			Run_Action_StepInto->setObjectName("stepInto");
			Run_Action_StepOver = RunMenu->addAction(VITR("YSS::menu.run.stepOver"));
			Run_Action_StepOver->setObjectName("stepOver");
			Run_Action_Suspend = RunMenu->addAction(VITR("YSS::menu.run.suspend"));
			Run_Action_Suspend->setObjectName("suspend");
			Run_Action_Resume = RunMenu->addAction(VITR("YSS::menu.run.resume"));
			Run_Action_Resume->setObjectName("resume");

			QObject::connect(Run_Run_Run, &QAction::triggered, q, &MainWinMenu::run_run_run);
			QObject::connect(Run_Run_Debug, &QAction::triggered, q, &MainWinMenu::run_run_debug);
			QObject::connect(Run_Run_Stop, &QAction::triggered, q, &MainWinMenu::run_run_stop);
			QObject::connect(Run_Run_Restart, &QAction::triggered, q, &MainWinMenu::run_run_restart);
			QObject::connect(Run_Build_Build, &QAction::triggered, q, &MainWinMenu::run_build_build);
			QObject::connect(Run_Build_Rebuild, &QAction::triggered, q, &MainWinMenu::run_build_rebuild);
			QObject::connect(Run_Build_Clean, &QAction::triggered, q, &MainWinMenu::run_build_clean);
			QObject::connect(Run_Action_StepInto, &QAction::triggered, q, &MainWinMenu::run_action_stepInto);
			QObject::connect(Run_Action_StepOver, &QAction::triggered, q, &MainWinMenu::run_action_stepOver);
			QObject::connect(Run_Action_Suspend, &QAction::triggered, q, &MainWinMenu::run_action_suspend);
			QObject::connect(Run_Action_Resume, &QAction::triggered, q, &MainWinMenu::run_action_resume);
		}

		void initViewMenu() {
			ViewMenu = MenuBar->addMenu(VITR("YSS::menu.view.title"));
			ViewMenu->setObjectName("view");
			View_ResourceBrowser = ViewMenu->addAction(VITR("YSS::menu.view.resourceBrowser"));
			View_ResourceBrowser->setObjectName("resourceBrowser");
			View_ResourceBrowser->setCheckable(true);
			View_MessageViewer = ViewMenu->addAction(VITR("YSS::menu.view.messageViewer"));
			View_MessageViewer->setObjectName("messageViewer");
			View_MessageViewer->setCheckable(true);
			View_PluginTools = ViewMenu->addMenu(VITR("YSS::menu.view.pluginTools"));
			View_PluginTools->setObjectName("pluginTools");

			QObject::connect(View_ResourceBrowser, &QAction::triggered, q, &MainWinMenu::view_resourceBrowser);
			QObject::connect(View_MessageViewer, &QAction::triggered, q, &MainWinMenu::view_messageViewer);
			QObject::connect(View_PluginTools, &QMenu::aboutToShow, q, &MainWinMenu::onPluginToolMenuAboutToShow);
		}
	};

	MainWinMenu::MainWinMenu(MainWin* parent) {
		d = new MainWinMenuPrivate();
		d->q = this;
		d->Parent = parent;
		this->setMaximumHeight(32);

		d->Layout = new QHBoxLayout(this);
		d->Layout->setContentsMargins(10, 0, 10, 0);
		d->MenuBar = new QMenuBar(this);
		d->Layout->addWidget(d->MenuBar);
		d->ProjectNameLabel = new Visindigo::Widgets::BorderLabel(this);
		d->ProjectNameLabel->setText(GlobalValue::getCurrentProject()->getProjectName());
		d->Layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		d->Layout->addWidget(d->ProjectNameLabel);

		d->initFileMenu();
		d->initEditMenu();
		d->initRunMenu();
		d->initViewMenu();
	}

	MainWinMenu::~MainWinMenu() {
		delete d;
	}

	void MainWinMenu::setShortcutTip(const QString& actionObjectName, const QString& tip) {
		QObject* child = Visindigo::Utility::QtSSHelper::findChildObject(d->MenuBar, actionObjectName);
		if (child) {
			QString rawText;
			QAction* action = qobject_cast<QAction*>(child);
			if (action) {
				rawText = action->text();
			}
			else {
				QMenu* menu = qobject_cast<QMenu*>(child);
				if (menu) {
					rawText = menu->title();
				}
				else {
					return;
				}
			}
			qint32 leftParenIndex = rawText.lastIndexOf('(');
			qint32 rightParenIndex = rawText.lastIndexOf(')');
			if (leftParenIndex != -1 && rightParenIndex != -1 && rightParenIndex > leftParenIndex) {
				rawText = rawText.left(leftParenIndex).trimmed();
			}
			rawText += QString(" (%1)").arg(tip);
			if (action) {
				action->setText(rawText);
			}
			else {
				QMenu* menu = qobject_cast<QMenu*>(child);
				menu->setTitle(rawText); // menu has been checked previously.
			}
		}
	}

	void MainWinMenu::setShortcutTips(const QMap<QString, QString>& tips) {
		for (auto it = tips.begin(); it != tips.end(); ++it) {
			setShortcutTip(it.key(), it.value());
		}
	}

	void MainWinMenu::file_file_new() {
		d->Parent->openNewFileWindow();
	}

	void MainWinMenu::file_file_open() {
		d->Parent->openFile();
	}

	void MainWinMenu::file_file_save() {
		d->Parent->saveCurrentFocusedFile();
	}

	void MainWinMenu::file_file_saveAs() {
		d->Parent->saveCurrentFocusedFileAs();
	}

	void MainWinMenu::file_file_saveAll() {
		d->Parent->saveAllFiles();
	}

	void MainWinMenu::file_file_projectConfig() {
		QString ysspFilePath = YSS::GlobalValue::getCurrentProject()->getProjectConfigPath();
		QDir projectDir(ysspFilePath);
		YSSFSM->openFile(projectDir.absoluteFilePath(""));
	}

	void MainWinMenu::file_file_showInExplorer() {
		QString projectFolder = YSS::GlobalValue::getCurrentProject()->getProjectFolder();
		Visindigo::Utility::FileUtility::openExplorer(projectFolder);
	}

	void MainWinMenu::file_program_backToHome() {
		d->Parent->backToHome();
	}

	void MainWinMenu::file_program_terminal() {
		auto terminal = VIApp->getVirtualTerminal();
		if (terminal) {
			terminal->show();
		}
	}

	void MainWinMenu::file_program_pluginAndPreferences() {
		auto widget = new Visindigo::Widgets::PluginManageWidget();
		widget->setAttribute(Qt::WA_DeleteOnClose);
		widget->setWindowModality(Qt::ApplicationModal);
		widget->setWindowFlags(widget->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
		widget->show();
	}

	void MainWinMenu::file_program_about() {
		QMessageBox::about(YSS::Editor::MainWin::getInstance(),
			VITR("YSS::editor.about.title"),
			VITR("YSS::editor.about.message").arg(VIApp->getMainPlugin()->getPluginVersion().toString()));
	}

	void MainWinMenu::file_program_documentation() {
		d->Parent->help();
	}

	void MainWinMenu::file_program_exit() {
		d->Parent->close();
	}

	void MainWinMenu::edit_undo() {
		auto currentWidget = d->Parent->getFileEditWidgetArea()->getCurrentWidget();
		if (currentWidget) {
			currentWidget->undo();
		}
	}

	void MainWinMenu::edit_redo() {
		auto currentWidget = d->Parent->getFileEditWidgetArea()->getCurrentWidget();
		if (currentWidget) {
			currentWidget->redo();
		}
	}

	void MainWinMenu::edit_cut() {
		auto currentWidget = d->Parent->getFileEditWidgetArea()->getCurrentWidget();
		if (currentWidget) {
			currentWidget->cut();
		}
	}

	void MainWinMenu::edit_copy() {
		auto currentWidget = d->Parent->getFileEditWidgetArea()->getCurrentWidget();
		if (currentWidget) {
			currentWidget->copy();
		}
	}

	void MainWinMenu::edit_paste() {
		auto currentWidget = d->Parent->getFileEditWidgetArea()->getCurrentWidget();
		if (currentWidget) {
			currentWidget->paste();
		}
	}

	void MainWinMenu::edit_selectAll() {
		auto currentWidget = d->Parent->getFileEditWidgetArea()->getCurrentWidget();
		if (currentWidget) {
			currentWidget->selectAll();
		}
	}

	void MainWinMenu::edit_findAndReplace() {
		auto currentWidget = d->Parent->getFileEditWidgetArea()->getCurrentWidget();
		auto textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(currentWidget);
		if (textEdit) {
			textEdit->showFindAndReplace();
		}
	}

	void MainWinMenu::run_run_run() {
		YSSDSR->run(false);
	}

	void MainWinMenu::run_run_debug() {
		YSSDSR->run(true);
	}

	void MainWinMenu::run_run_stop() {
		YSSDSR->stop();
	}

	void MainWinMenu::run_run_restart() {
		YSSDSR->stop();
		YSSDSR->run(false);
	}

	void MainWinMenu::run_build_build() {
		YSSDSR->build(false);
	}

	void MainWinMenu::run_build_rebuild() {
		YSSDSR->build(true);
	}

	void MainWinMenu::run_build_clean() {
		YSSDSR->clean();
	}

	void MainWinMenu::run_action_stepInto() {
		YSSDSR->stepInto();
	}

	void MainWinMenu::run_action_stepOver() {
		YSSDSR->stepOver();
	}

	void MainWinMenu::run_action_suspend() {
		YSSDSR->suspend();
	}

	void MainWinMenu::run_action_resume() {
		YSSDSR->contiune();
	}

	void MainWinMenu::view_resourceBrowser(bool checked) {
		d->Parent->getResourceBrowser()->setVisible(checked);
	}

	void MainWinMenu::view_messageViewer(bool checked) {
		//d->Parent->getMessageViewer()->setVisible(checked);
	}

	void MainWinMenu::onPluginToolMenuAboutToShow() {
		QMap<QString, QString> pluginTools = YSSTWM->getRegisteredToolWidgets();
		if (pluginTools.isEmpty()) {
			auto action = d->View_PluginTools->addAction(VITR("YSS::menu.view.noTools"));
			action->setEnabled(false);
			d->PluginToolsMap = pluginTools;
			d->PluginToolsActionIDMap.clear();
			return;
		}
		if (pluginTools != d->PluginToolsMap){
			for (auto action : d->PluginToolsActionIDMap.keys()) {
				d->View_PluginTools->removeAction(action);
			}
			d->PluginToolsMap = pluginTools;
			d->PluginToolsActionIDMap.clear();
			for (auto key : pluginTools.keys()) {
				auto action = d->View_PluginTools->addAction(VI18N(pluginTools[key]));
				action->setCheckable(true);
				d->PluginToolsActionIDMap[action] = key;
				QObject::connect(action, &QAction::triggered, this, [this, action]() {
					QString toolID = d->PluginToolsActionIDMap[action];
					bool checked = action->isChecked();
					view_pluginTools(toolID, checked);
				});
			}
		}
	}

	void MainWinMenu::view_pluginTools(const QString& toolID, bool checked) {
		auto widget = YSSTWM->getToolWidget(toolID);
		if (widget && not checked) {
			widget->close();
			return;
		}
		YSSTWM->openToolWidget(toolID);
	}
}