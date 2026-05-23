#pragma once
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

	class MainWin : public QFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		QWidget* CentralWidget = nullptr;
		ResourceBrowser* Browser = nullptr;
		FileEditWidgetArea* Editors = nullptr;
		ToolWidgetArea* Tools = nullptr;
		QVBoxLayout* MainLayout = nullptr;
		QHBoxLayout* Layout = nullptr;
		RenameDialog* RenameDlg = nullptr;
		MainWinMenu* Menu = nullptr;
		BottomInfoWidget* BottomFrame = nullptr;
		bool closeForBack = false;
		static MainWin* Instance;
	public:
		MainWin();
		virtual ~MainWin();
		static MainWin* getInstance();
		ResourceBrowser* getResourceBrowser();
		FileEditWidgetArea* getFileEditWidgetArea();
		void saveAllFiles();
		void saveProject();
		void backToHome();
		void onFileEditOpened(const QString& filePath);
		void onToolWidgetOpened(const QString& toolWidgetID);
		void saveCurrentFocusedFile();
		void saveCurrentFocusedFileAs(QString rawFilePath = "");
		void openFile(const QString& path = "");
		void openNewFileWindow();
		void help();
		void setMenuShortcutTips();
	public:
		virtual void onThemeChanged() override;
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
	};
}