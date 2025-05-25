#include "../ProjectWin.h"
#include "../ProjectInfoWidget.h"
#include <General/Version.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>

namespace YSS::ProjectPage {
	ProjectWin::ProjectWin() :QFrame() {
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
}