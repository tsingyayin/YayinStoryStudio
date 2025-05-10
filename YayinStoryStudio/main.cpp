#include <QtWidgets/QApplication>
#include "Editor/TitlePage/TitlePage.h"
#include <Utility/YamlConfig.h>
#include <General/YSSProject.h>
int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	YSS::TitlePage::TitlePage titlePage;
	titlePage.setScreen(QApplication::screenAt(QCursor::pos()));
	titlePage.show();
	YSSCore::General::YSSProject project;
	project.loadProject("./resource/repos/ase_test/config.yml");
	qDebug() << project.getProjectConfig()->keys("Project");
	qDebug() << project.getProjectConfig()->keys("Project");
	qDebug() << project.getProjectName();
	qDebug() << project.getProjectDescription();
	qDebug() << project.getProjectDebugServerID();
	return a.exec();
}