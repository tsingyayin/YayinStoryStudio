#pragma once
#include <QObject>
#include <QMainWindow>
#include <Widgets/ThemeManager.h>
class QMenuBar;
class QMenu;
class QAction;
class QHBoxLayout;
namespace Visindigo::Widgets {
	class QuickMenu;
}
namespace YSS::Editor {
	class FileEditorArea;
	class ResourceBrowser;

	class MainWin : public QMainWindow, Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		QWidget* CentralWidget = nullptr;
		ResourceBrowser* Browser = nullptr;
		FileEditorArea* Editor = nullptr;
		QHBoxLayout* Layout = nullptr;
		Visindigo::Widgets::QuickMenu* Menu = nullptr;
		bool asked = false;
		static MainWin* Instance;
	public:
		MainWin();
		static MainWin* getInstance();
		ResourceBrowser* getResourceBrowser();
		FileEditorArea* getFileEditorArea();
		void backToProjectWin();
		virtual void onThemeChanged() override;
	public:
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
	private:
		void initMenu();
		bool checkProjectNeedToSave();
		void saveProject();
	};
}