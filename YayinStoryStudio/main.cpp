#include <QtWidgets/QApplication>
#include "Editor/TitlePage/TitlePage.h"
#include <Utility/YamlConfig.h>
#include <General/YSSProject.h>
#include <General/private//UpdateChainSyntaxTest.h>
#include <Utility/CodeDiff.h>
#include <QtCore/qfile.h>
int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	QFile oldFile("./resource/old.txt");
	QFile newFile("./resource/new.txt");
	oldFile.open(QIODevice::ReadOnly);
	newFile.open(QIODevice::ReadOnly);
	QTextStream oldStream(&oldFile);
	QTextStream newStream(&newFile);
	QStringList oldCode;
	QStringList newCode;
	while (!oldStream.atEnd()) {
		QString line = oldStream.readLine();
		oldCode.append(line);
	}
	while (!newStream.atEnd()) {
		QString line = newStream.readLine();
		newCode.append(line);
	}
	YSSCore::Utility::CodeDiff codeDiff;
	codeDiff.setOldCode(oldCode);
	codeDiff.setNewCode(newCode);
	codeDiff.compare();
	codeDiff.debugPrint();

	YSS::TitlePage::TitlePage titlePage;
	titlePage.setScreen(QApplication::screenAt(QCursor::pos()));
	titlePage.show();
	YSSCore::General::YSSProject project;
	YSSCore::General::YSSProject::createProject("./resource/repos/ase_test", "ase_test");
	project.loadProject("./resource/repos/ase_test/yssproj.yml");
	

	qDebug() << project.getProjectConfig()->keys("Project");
	qDebug() << project.getProjectConfig()->keys("Project");
	qDebug() << project.getProjectName();
	qDebug() << project.getProjectDescription();
	qDebug() << project.getProjectDebugServerID();
	YSSCore::__Private__::MyUpdateChain updateChain;
	return a.exec();
}