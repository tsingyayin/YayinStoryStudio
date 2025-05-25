#include <QtWidgets/qapplication.h>
#include <Utility/YamlConfig.h>
#include <General/YSSProject.h>
#include <General/private//UpdateChainSyntaxTest.h>
#include <Utility/CodeDiff.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <Utility/FileUtility.h>
#include <Utility/AsyncFunction.h>
#include <Editor/ConfigWidget.h>

void test_main(QApplication& a) {
	qDebug() << QDir::tempPath();
	qDebug() << QDir::currentPath();
	qDebug() << a.arguments();
	YSSCore::Utility::CodeDiff codeDiff;
	codeDiff.setOldCode(YSSCore::Utility::FileUtility::readLines("./resource/old.txt"));
	codeDiff.setNewCode(YSSCore::Utility::FileUtility::readLines("./resource/new.txt"));
	codeDiff.compare();
	codeDiff.debugPrint();

	YSSCore::General::YSSProject project;
	YSSCore::General::YSSProject::createProject("./resource/repos/ase_test", "ase_test");
	project.loadProject("./resource/repos/ase_test/yssproj.yml");


	qDebug() << project.getProjectConfig()->keys("Project");
	qDebug() << project.getProjectConfig()->keys("Project");
	qDebug() << project.getProjectName();
	qDebug() << project.getProjectDescription();
	qDebug() << project.getProjectDebugServerID();

	YSSAsync<int, int, int>([](int a, int b)->int {
		QThread::msleep(10000);
		return a + b;
		},
		{ 3, 4 },
		[](int rtn) {
			qDebug() << rtn;
		}
	);
}

void test_loaded() {
	YSSCore::Editor::ConfigWidget* configWidget = new YSSCore::Editor::ConfigWidget();
	configWidget->setAttribute(Qt::WA_DeleteOnClose);
	QString jsonStr = YSSCore::Utility::FileUtility::readAll("./resource/config_widget.json");
	configWidget->loadCWJson(jsonStr);
	configWidget->show();
}