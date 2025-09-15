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
	public:
		MainWin();
		virtual void closeEvent(QCloseEvent* event) override;
		void applyStyleSheet();
	private:
		void initMenu();
	private:
		Visindigo::Widgets::QuickMenu* Menu = nullptr;
		QMenuBar* MenuBar = nullptr;
		QMenu* FileMenu = nullptr;
		QAction* SaveAction = nullptr;
		QAction* SaveAsAction = nullptr;
	};
}