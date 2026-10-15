#ifndef YayinStoryStudio_Editor_MainEditor_MainWin_h
#define YayinStoryStudio_Editor_MainEditor_MainWin_h
#include <QObject>
#include <QtWidgets/qframe.h>
#include <Widgets/ThemeManager.h>
#include <Editor/FileServerManager.h>
class QMenuBar;
class QMenu;
class QAction;
class QHBoxLayout;
class QVBoxLayout;
class QLineEdit;
namespace Visindigo::Widgets {
	class QuickMenu;
}
namespace YSS::Editor {
	class FileEditWidgetArea;
	class ResourceBrowser;
	class RenameDialog;
	class MainWinMenu;
	class ToolWidgetArea;
	class BottomInfoWidget;
	class TreeLayoutWidget;

	class MainWin : public QFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		QWidget* CentralWidget = nullptr;
		QVBoxLayout* MainLayout = nullptr;
		QHBoxLayout* Layout = nullptr;
		RenameDialog* RenameDlg = nullptr;
		MainWinMenu* Menu = nullptr;
		TreeLayoutWidget* TreeLayout = nullptr;
		BottomInfoWidget* BottomFrame = nullptr;
		bool CloseForBack = false;
		YSSCore::Editor::FileEditWidget* FocusingFileEditWidgetNotTool = nullptr;
		static MainWin* Instance;
	private:
		YSS::Editor::FileEditWidgetArea* selectFileEditWidgetAreaForNewFile(YSSCore::Editor::FileEditWidget* widget);
	signals:
		void currentFileEditWidgetAreaChanged(FileEditWidgetArea* area);
		void currentFileEditWidgetChanged(YSSCore::Editor::FileEditWidget* widget);
		void currentFileEditWidgetChangedNotTool(YSSCore::Editor::FileEditWidget* widget);
	public:
		MainWin();
		virtual ~MainWin();
		static MainWin* getInstance();
		void saveAllFiles(bool silent = false);
		void saveProject(bool silent = false);
		void backToHome();
		void onFileEditOpened(const QString& filePath);
		void saveCurrentFocusedFile();
		void saveCurrentFocusedFileAs(QString rawFilePath = "");
		void openFileDialog(const QString& startPath = "");
		void openNewFileWindow(const QString& startPath = "");
		void help();
		void setMenuShortcutTips();
		FileEditWidgetArea* getLastFocusedFileEditArea() const;
		YSSCore::Editor::FileEditWidget* getCurrentFocusedFileEditWidget() const;
		YSSCore::Editor::FileEditWidget* getCurrentFocusedFileEditWidgetNotTool() const;
		void initTreeLayout();
	public slots:
		void onFileEditWidgetAreaCreated(FileEditWidgetArea* widget);
		void onFileEditWidgetAreaFocusIn(const QString& areaID);
		void onGlobalCurrentChanged(FileEditWidgetArea* area, YSSCore::Editor::FileEditWidget* widget);
	public:
		virtual void onThemeChanged() override;
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_MainWin_h
