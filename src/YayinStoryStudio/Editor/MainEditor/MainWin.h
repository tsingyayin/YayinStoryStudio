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
namespace Visindigo::Widgets {
	class QuickMenu;
}
namespace YSS::Editor {
	class StackWidgetArea;
	class ResourceBrowser;
	class RenameDialog;

	class MainWin : public QFrame, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		QWidget* CentralWidget = nullptr;
		ResourceBrowser* Browser = nullptr;
		StackWidgetArea* Editor = nullptr;
		QVBoxLayout* MainLayout = nullptr;
		QHBoxLayout* Layout = nullptr;
		RenameDialog* RenameDlg = nullptr;
		Visindigo::Widgets::QuickMenu* Menu = nullptr;
		
		bool closeForBack = false;
		static MainWin* Instance;
	public:
		MainWin();
		virtual ~MainWin();
		static MainWin* getInstance();
		ResourceBrowser* getResourceBrowser();
		StackWidgetArea* getStackWidgetArea();
		void saveAllFiles();
		void saveProject();
		void backToProjectWin();
		virtual void onThemeChanged() override;
		void onFileEditOpened(const QString& filePath);
		void saveAs(QString rawFilePath = "");
		void openFile();
		void help();
	public:
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
	private:
		void initMenu();
		
	};
}