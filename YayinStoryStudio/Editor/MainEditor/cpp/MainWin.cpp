#include "../MainWin.h"
#include "../TextWidget.h"
#include "../ResourceBrowser.h"
#include "../../GlobalValue.h"
namespace YSS::Editor {

	MainWin::MainWin() :QMainWindow() {
		this->resize(1024, 768);
		this->setWindowTitle("Yayin Story Studio");
		//ProxyWidget->setParent(this);
		CentralWidget = new QWidget(this);
		Layout = new QHBoxLayout(CentralWidget);
		QSplitter* splitter = new QSplitter(Qt::Horizontal, CentralWidget);
		Editor = new TextWidget(CentralWidget);
		Browser = new ResourceBrowser(CentralWidget);
		Browser->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
		Editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		Layout->addWidget(splitter);
		splitter->addWidget(Browser);
		splitter->addWidget(Editor);
		
		this->setCentralWidget(CentralWidget);
		
		connect(Browser, &ResourceBrowser::openFile, [this](const QString& path) {
			Editor->addTextEdit(path);
			});

		this->initMenu();
	}

	
	void MainWin::initMenu() {
		MenuBar = new QMenuBar(this);
		this->setMenuBar(MenuBar);
		FileMenu = MenuBar->addMenu(GlobalValue::getTr("General.File"));
		SaveAction = new QAction(GlobalValue::getTr("General.Save"), this);
		SaveAsAction = new QAction(GlobalValue::getTr("General.SaveAs"), this);
		FileMenu->addAction(SaveAction);
		FileMenu->addAction(SaveAsAction);
	}
}