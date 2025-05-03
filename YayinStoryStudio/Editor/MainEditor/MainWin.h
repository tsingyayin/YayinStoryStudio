#pragma once
#include <QtWidgets>
#include "PlayerProxy.h"
namespace YSS::Editor {
	class TextWidget;
	class ResourceBrowser;

	class MainWin : public QMainWindow {
		Q_OBJECT;
	private:	
		QWidget* CentralWidget = nullptr;
		ResourceBrowser* Browser = nullptr;
		TextWidget* Editor = nullptr;
		QHBoxLayout* Layout = nullptr;
	public:
		MainWin();
		virtual void closeEvent(QCloseEvent* event) override;
	private:
		void initMenu();
	private:
		QMenuBar* MenuBar = nullptr;
		QMenu* FileMenu = nullptr;
		QAction* SaveAction = nullptr;
		QAction* SaveAsAction = nullptr;

	};
}