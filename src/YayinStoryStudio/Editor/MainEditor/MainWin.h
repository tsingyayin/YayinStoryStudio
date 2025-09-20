#pragma once
#include <QObject>
#include <QMainWindow>

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

	class MainWin : public QMainWindow {
		Q_OBJECT;
	private:
		QWidget* CentralWidget = nullptr;
		ResourceBrowser* Browser = nullptr;
		FileEditorArea* Editor = nullptr;
		QHBoxLayout* Layout = nullptr;
		Visindigo::Widgets::QuickMenu* Menu = nullptr;
		bool asked = false;
	public:
		MainWin();
		ResourceBrowser* getResourceBrowser();
		FileEditorArea* getFileEditorArea();
		void backToProjectWin();
		void applyStyleSheet();
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