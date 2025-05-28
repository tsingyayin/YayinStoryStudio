#include "../ProjectWin.h"
#include "../ProjectInfoWidget.h"
#include <General/Version.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include "../../GlobalValue.h"
#include <Utility/JsonConfig.h>

namespace YSS::ProjectPage {
	ProjectWin::ProjectWin() :QFrame() {
		this->setWindowIcon(QIcon(":/yss/compiled/yssicon.png"));
		int width = GlobalValue::getConfig()->getInt("Window.Project.Width");
		int height = GlobalValue::getConfig()->getInt("Window.Project.Height");
		this->setMinimumSize(800, 600);
		this->resize(width, height);
		this->setWindowTitle("Welcome");
		if (GlobalValue::getConfig()->getBool("Window.Project.Maximized")) {
			this->showMaximized();
		}
		TitleLabel = new QLabel(this);
		TitleLabel->setText("Yayin Story Studio " + YSSCore::General::Version::getYSSVersion().toString());
		HistoryProjectArea = new QScrollArea(this);
		HistoryProjectWidget = new QWidget(HistoryProjectArea);
		HistoryProjectArea->setWidget(HistoryProjectWidget);
		ProjectTemplateArea = new QScrollArea(this);
		ProjectTemplateWidget = new QWidget(ProjectTemplateArea);
		ProjectTemplateArea->setWidget(ProjectTemplateWidget);
		OptionWidget = new QWidget(this);
		OpenFolderButton = new QPushButton(OptionWidget);
		CloneGitButton = new QPushButton(OptionWidget);
		ButtonLayout = new QVBoxLayout(OptionWidget);
		OptionWidget->setLayout(ButtonLayout);
		ButtonLayout->addWidget(OpenFolderButton);
		ButtonLayout->addWidget(CloneGitButton);
		InfoWidget = new ProjectInfoWidget(this);
		Layout = new QGridLayout(this);

		this->setLayout(Layout);
		Layout->addWidget(TitleLabel, 0, 0, 1, 2);
		Layout->addWidget(HistoryProjectArea, 1, 0, 1, 1);
		Layout->addWidget(ProjectTemplateArea, 1, 1, 1, 1);
		Layout->addWidget(InfoWidget, 2, 0, 1, 1);
		Layout->addWidget(OptionWidget, 2, 1, 1, 1);
	}

	void ProjectWin::closeEvent(QCloseEvent* event){
		YSSCore::Utility::JsonConfig* config = GlobalValue::getConfig();
		if (this->isMaximized()) {
			config->setBool("Window.Project.Maximized", true);
		}
		else {
			config->setInt("Window.Project.Width", this->width());
			config->setInt("Window.Project.Height", this->height());
			config->setBool("Window.Project.Maximized", false);
		}
	}
}