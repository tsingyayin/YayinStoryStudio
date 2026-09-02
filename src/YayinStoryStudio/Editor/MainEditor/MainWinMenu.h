#ifndef YayinStoryStudio_Editor_MainEditor_MainWinMenu_h
#define YayinStoryStudio_Editor_MainEditor_MainWinMenu_h
#include <QtWidgets/qframe.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qaction.h>
#include <Widgets/ThemeManager.h>
#include <Editor/FileServer.h>
namespace YSS::Editor {
	class MainWin;
	class MainWinMenuPrivate;
	class MainWinMenu :public QFrame , public Visindigo::Widgets::ColorfulWidget{
		Q_OBJECT;
	public:
		MainWinMenu(MainWin* parent);
		virtual ~MainWinMenu();
		void refreshToolMenu();
		void setShortcutTip(const QString& actionObjectName, const QString& tip);
		void setShortcutTips(const QMap<QString, QString>& tips);
	public:
		void file_file_new();
		void file_file_open();
		void file_file_save();
		void file_file_saveAll();
		void file_file_saveAs();
		void file_file_projectConfig();
		void file_file_showInExplorer();
		void file_program_backToHome();
		void file_program_pluginAndPreferences();
		void file_program_colorThemeSettings();
		void file_program_about();
		void file_program_documentation();
		void file_program_exit();
	public:
		void edit_undo();
		void edit_redo();
		void edit_cut();
		void edit_copy();
		void edit_paste();
		void edit_selectAll();
		void edit_findAndReplace();
	public:
		void run_run_run();
		void run_run_debug();
		void run_run_stop();
		void run_run_restart();
		void run_build_build();
		void run_build_rebuild();
		void run_build_clean();
		void run_action_stepInto();
		void run_action_stepOver();
		void run_action_suspend();
		void run_action_resume();
	public:
		void view_resourceBrowser(bool checked);
		void view_fullScreenToggle(bool checked);
		void view_pluginTools(YSSCore::Editor::FileServer* asToolServer, bool checked);
		void syncPluginToolMenu();
	public:
		void onEditMenuAboutToShow();
		void onPluginToolMenuAboutToShow();
		void onDebugServerChanged();
	public:
		virtual void onThemeChanged() override;
	private:
		MainWinMenuPrivate* d;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_MainWinMenu_h
